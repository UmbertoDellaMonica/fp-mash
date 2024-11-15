// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "CommandSketch.h"
#include "Sketch.h"
#include "SketchFingerPrint.h"
#include "sketchParameterSetup.h"
#include <iostream>

using std::cerr;
using std::endl;
using std::cout;
using std::string;
using std::vector;

namespace mash {

CommandSketch::CommandSketch() : Command() {
    name = "sketch";
    summary = "Create sketches (reduced representations for fast operations).";
    description = "Create a sketch file, which is a reduced representation of a sequence or set of sequences (based on min-hashes) that can be used for fast distance estimations. Inputs can be fasta or fastq files (gzipped or not), and \"-\" can be given to read from standard input. Input files can also be files of file names (see -l). For output, one sketch file will be generated, but it can have multiple sketches within it, divided by sequences or files (see -i). By default, the output file name will be the first input file with a '.msh' extension, or 'stdin.msh' if standard input is used (see -o).";
    argumentString = "<input> [<input>] ...";
    
    useOption("help");
    addOption("list", Option(Option::Boolean, "l", "Input", "List input. Lines in each <input> specify paths to sequence files, one per line.", ""));
    addOption("prefix", Option(Option::File, "o", "Output", "Output prefix (first input file used if unspecified). The suffix '.msh' will be appended.", ""));
    addOption("id", Option(Option::File, "I", "Sketch", "ID field for sketch of reads (instead of first sequence ID).", ""));
    addOption("comment", Option(Option::File, "C", "Sketch", "Comment for a sketch of reads (instead of first sequence comment).", ""));
    addOption("counts", Option(Option::Boolean, "M", "Sketch", "Store multiplicity of each k-mer in each sketch.", ""));
    addOption("fingerprint", Option(Option::Boolean, "fp", "Input", "Indicates that the input files are fingerprints instead of sequences.", "")); // Opzione Fingerprint!
    addOption("mash-ver", Option(Option::Boolean, "ms", "Input", "Sketch files contains only the first 1000 sorted vectors of fingerprints, simulating Mash algorithm on genomes. It only works if -fp is used.", "")); 
    useSketchOptions();
}

int CommandSketch::run() const {
    
    // Verifica se ci sono gli argomenti giusti 
    if (!checkArguments()) {
        return 1; // Ritorna un errore se i parametri non sono validi
    }

    // Recupera il valore della Fingerprint -fp nel caso questa è attiva   
    bool fingerprint = options.at("fingerprint").active; 
    bool sorting = options.at("mash-ver").active;


    if(fingerprint && sorting){
        return runFingerPrintSorted();
    }

    /**
     * Se fingerPrint è attivo viene eseguita una funzione totalmente diversa 
     */
    if(fingerprint && !sorting){
        return runFingerPrint();
    }

    int verbosity = 1; // options.at("silent").active ? 0 : options.at("verbose").active ? 2 : 1;
    

    bool list = options.at("list").active;

    // Dichiaro l'oggetto Parameters dello sketch 
    Sketch::Parameters parameters;

    // Recupero il parametro di counts 
    parameters.counts = options.at("counts").active;

    // Inizializzo i Parametri dello sketch 
    if (sketchParameterSetup(parameters, *(Command *)this))
    {
        return 1;
    }

    std::vector<std::string> files;
    // Recupero tutti i files che sono stati listati
    populateFiles(files, list);

    // Dichiaro lo sketch file 
    Sketch sketch;

    cout<< "Sono in questo punto del debugging! "<<endl;

    uint64_t lengthMax;
    double randomChance;
    int kMin;
    string lengthMaxName;
    int warningCount = 0;


    if (parameters.reads){

        sketch.initFromReads(files, parameters);
    
    }else{

        sketch.initFromFiles(files, parameters, verbosity);
    }

    if (getOption("id").active)
    {
        sketch.setReferenceName(0, getOption("id").argument);
    }

    if (getOption("comment").active)
    {
        sketch.setReferenceComment(0, getOption("comment").argument);
    }

    // Analyze Reference Lengths - settaggio delle impostazioni della reference 
    analyzeReferenceLengths(SketchFingerPrint(), sketch, parameters.warning, lengthMax, lengthMaxName, warningCount, randomChance, kMin, false );

    cout<< "Sono in questo punto del debugging dopo l'analisi ! "<<endl;


    // Determina il prefisso e il suffisso del file 
    std::string prefix = determinePrefixAndSuffix(arguments[0], fingerprint, SketchFingerPrint::Parameters(), parameters);

    // Effettua la scrittura del file per l'Indicizzazione di Capnp
    sketch.writeToCapnp(prefix.c_str());
    
    if ( warningCount > 0 && ! parameters.reads )
    {
    	warnKmerSize(parameters, *this, lengthMax, lengthMaxName, randomChance, kMin, warningCount);
    }

    cout<< "Sono in questo punto del debugging , ho catturato il valore del warnKmerSize e ho generato il file ! "<<endl;
    
    return 0;
}


int CommandSketch::runFingerPrint() const {

    cout << "I'm Here ! " << "... in sketch fingerprint" << endl;

    int verbosity = 1; // options.at("silent").active ? 0 : options.at("verbose").active ? 2 : 1;
    

    bool list = options.at("list").active;

    // Instanzio l'oggetto dei parametri della fingerprint 
    SketchFingerPrint::Parameters parametersFingerprint;
    // Attivo l'opzione del parametro "counts"
    parametersFingerprint.counts = options.at("counts").active;
    
    // Inizializza i parametri della fingerprint
    if (sketchParameterFingerPrintSetup(parametersFingerprint, *(Command *)this))
    {
        return 1;
    }


    std::vector<std::string> files;
    // Inserisco nel vettore tutti i file che devo 
    populateFiles(files, list);

    // Dichiaro l'oggetto - SketchFingerPrint
    SketchFingerPrint sketchFingerPrint;

    // Settings Reference 
    uint64_t lengthMax;
    double randomChance;
    int kMin;
    string lengthMaxName;
    int warningCount = 0;


    // Inizializzo il file di SketchFingerPrint
    sketchFingerPrint.initFromFingerprints(files, parametersFingerprint); // Nuova funzione per fingerprint

    // Recupera l'opzione "id" e se attiva la mette all'interno dello sketch 
    if (getOption("id").active)
    {
        sketchFingerPrint.setReferenceName(0, getOption("id").argument);
    }
    // Recupera l'opzione "comment" e se attiva la mette all'interno dello sketch 
    if (getOption("comment").active)
    {
        sketchFingerPrint.setReferenceComment(0, getOption("comment").argument);
    }

    // Analyze Reference Lengths - settaggio delle impostazioni della reference 
    analyzeReferenceLengths(sketchFingerPrint, Sketch(), parametersFingerprint.warning, lengthMax, lengthMaxName, warningCount, randomChance, kMin, true );

    // Determina il prefisso e il suffisso del file 
    std::string prefix = determinePrefixAndSuffix(arguments[0], true, parametersFingerprint, Sketch::Parameters());

    // Effettua la scrittura del file per l'Indicizzazione di Capnp
    sketchFingerPrint.writeToCapnpFingerPrint(prefix.c_str());

    // Applicazione di WarnKMerFingerPrint 
    /*if ( warningCount > 0 && ! parametersFingerprint.reads )
    {
    	warnKmerFingerPrintSize(parametersFingerprint, *this, lengthMax, lengthMaxName, randomChance, kMin, warningCount);
    }*/

    return 0; 
}

int CommandSketch::runFingerPrintSorted() const {

    cout << "I'm Here ! " << "... in sketch fingerprint sorted" << endl;

    int verbosity = 1; // options.at("silent").active ? 0 : options.at("verbose").active ? 2 : 1;
    

    bool list = options.at("list").active;

    // Instanzio l'oggetto dei parametri della fingerprint 
    SketchFingerPrint::Parameters parametersFingerprint;
    // Attivo l'opzione del parametro "counts"
    parametersFingerprint.counts = options.at("counts").active;
    
    // Inizializza i parametri della fingerprint
    if (sketchParameterFingerPrintSetup(parametersFingerprint, *(Command *)this))
    {
        return 1;
    }


    std::vector<std::string> files;
    // Inserisco nel vettore tutti i file che devo 
    populateFiles(files, list);

    // Dichiaro l'oggetto - SketchFingerPrint
    SketchFingerPrint sketchFingerPrint;

    // Settings Reference 
    uint64_t lengthMax;
    double randomChance;
    int kMin;
    string lengthMaxName;
    int warningCount = 0;


    // Inizializzo il file di SketchFingerPrint
    sketchFingerPrint.initFromFingerprintsSorted(files, parametersFingerprint); // Nuova funzione per fingerprint

    // Recupera l'opzione "id" e se attiva la mette all'interno dello sketch 
    if (getOption("id").active)
    {
        sketchFingerPrint.setReferenceName(0, getOption("id").argument);
    }
    // Recupera l'opzione "comment" e se attiva la mette all'interno dello sketch 
    if (getOption("comment").active)
    {
        sketchFingerPrint.setReferenceComment(0, getOption("comment").argument);
    }

    // Analyze Reference Lengths - settaggio delle impostazioni della reference 
    analyzeReferenceLengths(sketchFingerPrint, Sketch(), parametersFingerprint.warning, lengthMax, lengthMaxName, warningCount, randomChance, kMin, true );

    // Determina il prefisso e il suffisso del file 
    std::string prefix = determinePrefixAndSuffix(arguments[0], true, parametersFingerprint, Sketch::Parameters());

    // Effettua la scrittura del file per l'Indicizzazione di Capnp
    sketchFingerPrint.writeToCapnpFingerPrint(prefix.c_str());

    // Applicazione di WarnKMerFingerPrint 
    /*if ( warningCount > 0 && ! parametersFingerprint.reads )
    {
    	warnKmerFingerPrintSize(parametersFingerprint, *this, lengthMax, lengthMaxName, randomChance, kMin, warningCount);
    }*/

    return 0; 
}


bool CommandSketch::checkArguments() const {
    /**
     * @brief Verifica la validità degli argomenti passati al comando.
     * 
     * Questo metodo controlla se la lista degli argomenti è vuota o se l'opzione "help" è attiva.
     * Se uno di questi casi è vero, il metodo chiama la funzione `print` per visualizzare 
     * un messaggio di aiuto e ritorna `false`, indicando che gli argomenti non sono validi.
     * 
     * @return `true` se gli argomenti sono validi e il comando può continuare; `false` altrimenti.
     * 
     * La verifica consiste nei seguenti passaggi:
     * 1. Controlla se la lista degli argomenti (`arguments`) è vuota.
     * 2. Controlla se l'opzione "help" (`options.at("help")`) è attiva.
     *    - `options.at("help").active` verifica se l'opzione "help" è stata specificata.
     * 
     * Se uno dei due controlli è positivo, il metodo:
     * 1. Chiama la funzione `print()` per visualizzare il messaggio di aiuto.
     * 2. Ritorna `false` per indicare che la verifica degli argomenti è fallita.
     * 
     * Se entrambi i controlli sono negativi, il metodo ritorna `true` per indicare che gli argomenti
     * sono validi e il comando può proseguire con l'esecuzione.
     * 
     * Questo metodo è privato e viene utilizzato internamente nella classe `CommandSketch` per 
     * assicurare che gli argomenti siano appropriati prima di eseguire il comando.
     */
    if (arguments.size() == 0 || options.at("help").active) {
        print();
        return false;
    }
    return true;
}


void CommandSketch::populateFiles(std::vector<std::string> &files, bool list) const {
    /**
     * @brief Popola il vettore di file in base agli argomenti e l'opzione 'list'.
     * 
     * Questo metodo scorre tutti gli argomenti passati al comando. Se l'opzione 'list' è attiva,
     * ogni argomento viene passato alla funzione `splitFile` per essere ulteriormente suddiviso
     * e i risultati vengono aggiunti al vettore `files`. Altrimenti, ogni argomento viene aggiunto
     * direttamente al vettore `files`.
     * 
     * @param files Riferimento al vettore di stringhe da popolare con i file.
     * @param list Booleano che indica se l'opzione 'list' è attiva.
     */
    for (int i = 0; i < arguments.size(); i++) {
        if (list) {
            splitFile(arguments[i], files);
        } else {
            files.push_back(arguments[i]);
        }
    }
}


std::string CommandSketch::determinePrefixAndSuffix(const std::string &arg, bool isFingerprint, const SketchFingerPrint::Parameters &parametersFingerprint, const Sketch::Parameters &parameters) const {
    /**
     * @brief Determina il prefisso e il suffisso per i file.
     * 
     * Questo metodo determina il prefisso in base all'opzione "prefix" e all'argomento passato.
     * Successivamente, aggiunge il suffisso appropriato in base ai parametri specificati.
     * 
     * @param arg L'argomento da cui determinare il prefisso.
     * @param isFingerprint Booleano che indica se usare i parametri di fingerprint.
     * @param parametersFingerprint Parametri per fingerprint.
     * @param parameters Parametri generali.
     * @return Una stringa che rappresenta il prefisso con il suffisso appropriato.
     */
    std::string prefix;
    if (options.at("prefix").argument.length() > 0) {
        prefix = options.at("prefix").argument;
    } else {
        if (arg == "-") {
            prefix = "stdin";
        } else {
            prefix = arg;
        }
    }

    std::string suffix;
    if (isFingerprint) {
        suffix = parametersFingerprint.windowed ? suffixFingerPrintSketchWindowed : suffixFingerPrintSketch;
        if (!hasSuffixFingerPrint(prefix, suffix)) {
            prefix += suffix;
        }
    } else {
        suffix = parameters.windowed ? suffixSketchWindowed : suffixSketch;
        if (!hasSuffix(prefix, suffix)) {
            prefix += suffix;
        }
    }

    return prefix;
}


void CommandSketch::analyzeReferenceLengths(const SketchFingerPrint& sketchFingerPrint, const Sketch& sketch,double warning, uint64_t& lengthMax, std::string& lengthMaxName, int& warningCount, double& randomChance, int& kMin, bool isFingerPrint) const {
    
    if(isFingerPrint){
        // Sketch FingerPrint - Reference 
        double lengthThreshold = (warning * sketchFingerPrint.getKmerSpace()) / (1. - warning);

        for (int i = 0; i < sketchFingerPrint.getReferenceCount(); i++) {
            uint64_t length = sketchFingerPrint.getReference(i).length;

            if (length > lengthThreshold) {
                if (warningCount == 0 || length > lengthMax) {
                    lengthMax = length;
                    lengthMaxName = sketchFingerPrint.getReference(i).name;
                    randomChance = sketchFingerPrint.getRandomKmerChance(i);
                    kMin = sketchFingerPrint.getMinKmerSize(i);
                }
                warningCount++;
            }
        }

    }else{
        // Sketch - Reference 
        double lengthThreshold = (warning * sketch.getKmerSpace()) / (1. - warning);

        for (int i = 0; i < sketch.getReferenceCount(); i++) {
            uint64_t length = sketch.getReference(i).length;

            if (length > lengthThreshold) {
                if (warningCount == 0 || length > lengthMax) {
                    lengthMax = length;
                    lengthMaxName = sketch.getReference(i).name;
                    randomChance = sketch.getRandomKmerChance(i);
                    kMin = sketch.getMinKmerSize(i);
                }
                warningCount++;
            }
        }
    }
}




} // namespace mash