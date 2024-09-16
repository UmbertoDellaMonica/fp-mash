// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "CommandPaste.h"
#include "Sketch.h"
#include <iostream>
#include "unistd.h"

using std::string;
using std::cerr;
using std::endl;

namespace mash {

// Costruttore per il comando "paste"
CommandPaste::CommandPaste()
: Command()
{
    name = "paste";
    summary = "Create a single sketch file from multiple sketch files.";
    description = "Create a single sketch file from multiple sketch files.";
    argumentString = "<out_prefix> <sketch> [<sketch>] ...";
    
    // Opzioni disponibili per questo comando
    useOption("help");
    // Opzione list per inserire un file .txt e leggere da quel file vari file di input 
    addOption("list", Option(Option::Boolean, "l", "", "Input files are lists of file names.", ""));
    // Opzione fingerPrint per inserire file di tipo fingerprint con formato .txt
    addOption("fingerPrint", Option(Option::Boolean, "fp" ,"","Insert fingerprint files are lists of file names.",""));
    // Opzione per indicare che questo file è un file di Output 
    addOption("output",Option(Option::Boolean,"o","","Insert -o to indicate the name and path for the output file. Take this option as the last one after -fp or -l ",""));

}


bool fileExists(const std::string& filename) {
    return access(filename.c_str(), F_OK) != -1;
}



// Funzione principale per eseguire il comando
int CommandPaste::run() const
{

    bool output = options.at("output").active; // Verifica se l'opzione è attiva 

    bool list = options.at("list").active; // Verifica se l'opzione list è attiva
    
    bool fingerPrint = options.at("fingerPrint").active; // Verifica se l'opzione fingerprint è attiva 

    // Combination of two option it is not allowed 
    if(list && fingerPrint){
        
        cerr << "ERROR: The options -l and -fp are incompatible." << endl;
        return 1;
    }

    // Se il numero di argomenti è insufficiente o se l'opzione help è attiva, mostra l'help
    if ( arguments.size() < 2 || options.at("help").active )
    {
        print();
        return 0;
    }
    
    std::vector<string> files; // Vettore per memorizzare i nomi dei file
        
    string out;

    /**
     * Se l'opzione Output viene specificata all'interno del comando è necessario inserire il file di output alla fine come argomento 
     * ** L'opzione va specificata 
     * ** Se l'opzione non viene specificata allora l'output file deve andare all'inizio come argomento 
     */

    if(output){
    // Itera attraverso gli argomenti
        for ( int i = 0; i < arguments.size()-1; i++ ){
            
            if ( list )
            {
                // Se l'opzione list è attiva, leggi i nomi dei file da un file di testo
                //splitFile(arguments[i], files);
                // TODO : Modifica questa parte 
                files.push_back(arguments[i]);
            }

            else if ( fingerPrint ){
                
                files.push_back(arguments[i]);

            }
            else
            {
                // Altrimenti, aggiungi direttamente il nome del file al vettore
                files.push_back(arguments[i]);
            }
        }
        // Se l'opzione Output è disponibile 
        out = arguments[arguments.size()-1];
    }else{


         for ( int i = 1; i < arguments.size(); i++ ){
            
            if ( list )
            {
                // Se l'opzione list è attiva, leggi i nomi dei file da un file di testo
                //splitFile(arguments[i], files);
                // TODO : Modifica questa parte 
                files.push_back(arguments[i]);
            }

            else if ( fingerPrint ){
                
                files.push_back(arguments[i]);

            }
            else
            {
                // Altrimenti, aggiungi direttamente il nome del file al vettore
                files.push_back(arguments[i]);
            }
        }
    
        out = arguments[0];

    }
    
    Sketch sketch; // Oggetto Sketch per gestire gli sketch
    std::vector<string> filesGood; // Vettore per memorizzare i file validi
    Sketch::Parameters parameters; // Parametri per l'oggetto Sketch
    parameters.parallelism = 1; // Imposta il parallelismo a 1
    
    // Deve iterare su file che hanno suffisso sia .txt e sia .msh 
    if(fingerPrint){


        /**
         * Se vi è l'opzione fingerprint abbiamo 2 casi :
         * - 1 Il caso principale è quello di inserire i file fingerprint del tipo .txt con l'opzione -fp
         *      In tal caso, verifichiamo se ci sono i corrispettivi file .msh dei vari .txt 
         *      Se il file non esiste -> si ritorna 1 e si manda un messaggio di errore su quel file "Effettuare prima lo sketch del file fingerPrint e poi eseguire la paste"
         *                              altrimenti -> facciamo il replace della stringa all'interno del vettore che contiene i file
         * 
         * - 2 Il secondo caso prevede la verifica dell'esistenza dei file .txt dal file .msh con l'opzione di fingerprint 
         *     Se l'opzione -fp e i file sono in formato .msh -> si vanno a verificare che esistano i corrispettivi file .txt
         *                                              altrimenti -> stampiamo un messaggio di errore e ritorniamo 1 
         */

        for (int i = 0; i < files.size(); i++){
        string & file = files[i];

        // Verifica se il file ha il suffisso corretto
        if ((!hasSuffix(file, suffixFingerprint)) && (!hasSuffix(file, suffixSketch)))
        {
            cerr << "ERROR: The file \"" << file << "\" does not look like a fingerprint or sketch." << endl;
            return 1;
        }

        // Se il file è un .txt, verifica se esiste un file .msh corrispondente
        if (hasSuffix(file, ".txt")) {
            std::string mshFile = file.substr(0, file.size() - 4) + ".msh"; // sostituisce .txt con .msh

            if (fileExists(mshFile)) {
                // Se esiste il file .msh corrispondente, sostituisci il file corrente con quello .msh
                file = mshFile;
            }else{
                cerr << "ERROR: The file \"" << mshFile << "\" does not exist but is required. Do the command sketch before doing this operation " << endl;
                return 1;
            }
        }
        // Se il file è un .msh, verifica se esiste un file .txt corrispondente
        else if (hasSuffix(file, ".msh")) {
            std::string txtFile = file.substr(0, file.size() - 4) + ".txt"; // sostituisce .msh con .txt

            if (fileExists(txtFile)) {
                // Se esiste il file .txt corrispondente, prosegui normalmente
            } else {
                cerr << "ERROR: The file \"" << txtFile << "\" does not exist but is required." << endl;
                return 1;
            }
        }

        // Aggiungi il file al vettore dei file validi
        filesGood.push_back(file);
    }


    }else{

        // List Option -l or nothing as option 


        // Itera attraverso i file forniti
        for ( int i = 0; i < files.size(); i++ )
        {
            const string & file = files[i];
            // Verifica se il file ha il suffisso corretto
            if ( ! hasSuffix(file, suffixSketch) )
            {
                cerr << "ERROR: The file \"" << file << "\" does not look like a sketch." << endl;
                return 1;
            }
            
            // Aggiungi il file al vettore dei file validi
            filesGood.push_back(file);
        }
    }
    
        sketch.initFromFiles(filesGood, parameters);



// --------------------- Operazioni sul file di Output ----------------------------------//


    // Recupera il file di Output che viene visualizzato come primo argomento

    
    
    // Verifica sul file di Output 


    // Aggiungi il suffisso al file di output se non è presente
    if ( ! hasSuffix(out, suffixSketch) )
    {
        out += suffixSketch;
    }

    // Verifica se il file di output esiste già
    if( fileExists(out) )
    {
        cerr << "ERROR: \"" << out << "\" exists; remove to write." << endl;
        exit(1);
    }
    
    cerr << "Writing " << out << "..." << endl; // Messaggio di log
    sketch.writeToCapnp(out.c_str()); // Scrivi l'oggetto Sketch nel file di output
    
    return 0; // Termina con successo
}




} // namespace mash
