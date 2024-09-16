// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "CommandContain.h"
#include "Sketch.h"
#include <iostream>
#include <zlib.h>
#include "ThreadPool.h"
#include "sketchParameterSetup.h"
#include <math.h>

using namespace::std;

namespace mash {

// Costruttore della classe CommandContain, che deriva dalla classe Command
CommandContain::CommandContain()
: Command()  // Costruttore della classe base (Command)
{
    // Nome del comando
    name = "contain";
    
    // Breve descrizione del comando, che verrà mostrata come sommario
    summary = "Estimate the containment of query sequences within references.";
    
    // Descrizione più dettagliata del comando, che spiega cosa fa il comando 'contain'
    description = "Estimate the containment of each query file (or sequence with -i) in the reference. "
                  "Both the reference and queries can be fasta or fastq, gzipped or not, or mash sketch files (.msh) "
                  "with matching k-mer sizes. Query files can also be files of file names (see -l). "
                  "The score is the number of intersecting min-hashes divided by the query set size. "
                  "The output format is [score, error-bound, reference-ID, query-ID].";
    
    // Stringa che descrive gli argomenti attesi dal comando, in questo caso i file di riferimento e query
    argumentString = "<reference> <query> [<query>] ...";
    
    // Aggiunta di un'opzione al comando:
    // - "list" è un'opzione booleana (vero/falso) che indica se l'input contiene una lista di file di sequenze.
    // - Viene specificato che questa opzione si riferisce all'input ("Input").
    // - La descrizione dell'opzione spiega che se attiva, ogni file di query contiene una lista di file di sequenze, una per linea.
    // - Il file di riferimento non è influenzato da questa opzione.
    addOption("list", Option(Option::Boolean, "l", "Input", "List input. Each query file contains a list of sequence files, one per line. The reference file is not affected.", ""));
    
    // Aggiunta di un'altra opzione al comando:
    // - "errorThreshold" è un'opzione numerica che specifica la soglia dell'errore massimo accettabile per la segnalazione dei valori di score.
    // - Questa opzione è categorizzata sotto l'output ("Output").
    // - La descrizione spiega che aumentando la dimensione dello sketch del riferimento, si può generalmente ridurre il bound dell'errore.
    // - Il valore predefinito di questa opzione è "0.05".
    addOption("errorThreshold", Option(Option::Number, "e", "Output", "Error bound threshold for reporting scores values. Error bounds can generally be increased by increasing the sketch size of the reference.", "0.05"));
    
    // Aggiunta dell'opzione "help", ereditata dalla classe base Command.
    // Questa opzione generalmente fornisce informazioni sull'uso del comando.
    useOption("help");
    
    // Aggiunta delle opzioni relative agli sketch, che includono parametri come la dimensione del k-mer.
    useSketchOptions();
}


/**Inizio del Programma: La funzione run inizia controllando se ci sono abbastanza argomenti passati alla linea di comando e se l'opzione "help" è attiva. Se necessario, stampa il messaggio di aiuto e termina.

Setup dei Parametri: I parametri relativi alla soglia di errore e ai thread vengono configurati. Se il setup fallisce, il programma termina con un errore.

Gestione dello Sketch di Riferimento: Se il file di riferimento è già uno sketch, alcuni parametri vengono ereditati dallo sketch stesso. Altrimenti, il file viene "sketched" (una rappresentazione compatta viene creata) e i parametri vengono impostati di conseguenza.

Pool di Thread: Viene creato un ThreadPool per eseguire in parallelo il calcolo di containment. Questo permette al programma di gestire grandi dataset in modo efficiente.

Gestione delle Query: I file di query vengono preparati per essere processati in parallelo. Se l'opzione "list" è attiva, i file vengono divisi in più file di sequenze.

Calcolo delle Coppie: Viene calcolato il numero di coppie di riferimenti e query da processare per ogni thread. Viene imposto un limite massimo per evitare che un singolo thread gestisca troppe coppie.

Esecuzione del Thread Pool: Le coppie di query e riferimenti vengono assegnate ai thread per l'elaborazione. L'output viene scritto appena è disponibile, assicurando che non ci siano ritardi nel processo di output.

Termine dell'Esecuzione: La funzione si assicura che tutti i thread siano completati e che tutto l'output sia stato scritto prima di terminare con successo. */

int CommandContain::run() const
{
    // Se il numero di argomenti è inferiore a 2 o l'opzione "help" è attiva, stampa l'aiuto e termina l'esecuzione.
    if ( arguments.size() < 2 || options.at("help").active )
    {
        print();  // Stampa il messaggio di aiuto.
        return 0; // Esce dalla funzione con codice di successo.
    }
    
    // Ottiene il numero di thread da usare dalle opzioni e verifica se l'opzione "list" è attiva.
    int threads = options.at("threads").getArgumentAsNumber();
    bool list = options.at("list").active;
    
    // Imposta i parametri per lo sketching.
    Sketch::Parameters parameters;
    parameters.error = options.at("errorThreshold").getArgumentAsNumber();  // Imposta la soglia di errore.

    // Configura i parametri per lo sketch. Se fallisce, ritorna con errore.
    if ( sketchParameterSetup(parameters, *(Command *)this) )
    {
    	return 1; // Esce dalla funzione con codice di errore.
    }
    
    // Crea un oggetto Sketch per il riferimento.
    Sketch sketchRef;
    const string & fileReference = arguments[0];  // Il primo argomento è il file di riferimento.
    
    // Verifica se il file di riferimento è uno sketch (controlla l'estensione).
    bool isSketch = hasSuffix(fileReference, suffixSketch);
    
    if ( isSketch )
    {
        // Se il file di riferimento è uno sketch, alcune opzioni non possono essere utilizzate.
        if ( options.at("kmer").active )
        {
            cerr << "ERROR: The option " << options.at("kmer").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        
        if ( options.at("noncanonical").active )
        {
            cerr << "ERROR: The option " << options.at("noncanonical").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        
        if ( options.at("protein").active )
        {
            cerr << "ERROR: The option " << options.at("protein").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
        }
        
        if ( options.at("alphabet").active )
        {
            cerr << "ERROR: The option " << options.at("alphabet").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
        }
    }
    else
    {
        // Se non è uno sketch, si esegue lo sketching del file di riferimento.
        cerr << "Sketching " << fileReference << " (provide sketch file made with \"mash sketch\" to skip)...";
    }
    
    // Aggiunge il file di riferimento a un vettore di stringhe per l'inizializzazione dello sketch.
    vector<string> refArgVector;
    refArgVector.push_back(fileReference);
    
    // Inizializza lo sketch del riferimento dal file.
    sketchRef.initFromFiles(refArgVector, parameters);
    
    if ( isSketch )
    {
        // Se il riferimento è uno sketch, eredita i parametri dal file di sketch.
        parameters.minHashesPerWindow = sketchRef.getMinHashesPerWindow();
        parameters.kmerSize = sketchRef.getKmerSize();
        parameters.noncanonical = sketchRef.getNoncanonical();
        parameters.preserveCase = sketchRef.getPreserveCase();
        parameters.seed = sketchRef.getHashSeed();
        
        // Ottiene l'alfabeto dallo sketch e lo imposta nei parametri.
        string alphabet;
        sketchRef.getAlphabetAsString(alphabet);
        setAlphabetFromString(parameters, alphabet.c_str());
    }
    else
    {
        // Completa l'output del processo di sketching.
	    cerr << "done.\n";
    }
    
    // Crea un pool di thread per gestire l'esecuzione parallela del calcolo di containment.
    ThreadPool<ContainInput, ContainOutput> threadPool(contain, threads);
    
    // Crea un vettore di stringhe per i file di query.
    vector<string> queryFiles;
    
    // Popola il vettore di file di query.
    for ( int i = 1; i < arguments.size(); i++ )
    {
        if ( list )
        {
            // Se l'opzione "list" è attiva, divide il file in più file di sequenze.
            splitFile(arguments[i], queryFiles);
        }
        else
        {
            queryFiles.push_back(arguments[i]);  // Aggiunge il file al vettore.
        }
    }
    
    // Inizializza lo sketch della query.
    Sketch sketchQuery;
    sketchQuery.initFromFiles(queryFiles, parameters, 0, true, true);
    
    // Calcola il numero totale di coppie di riferimenti e query.
    uint64_t pairCount = sketchRef.getReferenceCount() * sketchQuery.getReferenceCount();
    
    // Determina il numero di coppie da processare per thread.
    uint64_t pairsPerThread = pairCount / parameters.parallelism;
    
    if ( pairsPerThread == 0 )
    {
    	pairsPerThread = 1;  // Assicura che ci sia almeno una coppia per thread.
    }
    
    // Limita il numero massimo di coppie per thread.
    static uint64_t maxPairsPerThread = 0x1000;
    
    if ( pairsPerThread > maxPairsPerThread )
    {
        pairsPerThread = maxPairsPerThread;  // Limita le coppie per thread.
    }
    
    // Calcola i valori di iFloor e iMod per dividere le coppie tra i thread.
    uint64_t iFloor = pairsPerThread / sketchRef.getReferenceCount();
    uint64_t iMod = pairsPerThread % sketchRef.getReferenceCount();
    
    // Avvia l'elaborazione delle coppie di riferimenti e query sui thread disponibili.
    for ( uint64_t i = 0, j = 0; i < sketchQuery.getReferenceCount(); i += iFloor, j += iMod )
    {
        if ( j >= sketchRef.getReferenceCount() )
        {
            // Se j supera il numero di riferimenti, lo resetta e passa alla prossima query.
            if ( i == sketchQuery.getReferenceCount() - 1 )
            {
                break;
            }
            
            i++;
            j -= sketchRef.getReferenceCount();
        }
        
        // Avvia un nuovo lavoro nel thread pool con i parametri calcolati.
		threadPool.runWhenThreadAvailable(new ContainInput(sketchRef, sketchQuery, j, i, pairsPerThread, parameters));
	
        // Scrive l'output quando disponibile dal thread pool.
		while ( threadPool.outputAvailable() )
		{
			writeOutput(threadPool.popOutputWhenAvailable(), parameters.error);
		}
    }
    
    // Assicura che tutti i thread siano completati e scrive l'output finale.
    while ( threadPool.running() )
    {
        writeOutput(threadPool.popOutputWhenAvailable(), parameters.error);
    }
    
    return 0;  // Termina l'esecuzione con codice di successo.
}



/**Inizializzazione degli Indici: Questi indici sono utilizzati per scorrere attraverso gli sketch di query (i) e di riferimento (j) mentre vengono processate le coppie di risultati.

Iterazione sui Risultati: Il ciclo for percorre tutte le coppie di risultati (pairCount) e si assicura che ci siano ancora sketch di query da confrontare.

Accesso alla Coppia di Risultati: La struttura PairOutput rappresenta il risultato di una singola comparazione tra uno sketch di query e uno di riferimento.

Filtraggio per Errore: Solo le comparazioni che hanno un errore inferiore o uguale al valore fornito vengono considerate per l'output.

Scrittura dell'Output: Se la coppia è valida (cioè ha un errore accettabile), i risultati vengono stampati, mostrando il punteggio, l'errore e i nomi degli sketch comparati.

Avanzamento degli Indici: Dopo ogni comparazione, l'indice del riferimento (j) viene incrementato. Quando tutti gli sketch di riferimento sono stati processati, l'indice viene reimpostato e si passa al prossimo sketch di query (i).

Pulizia della Memoria: Alla fine, l'oggetto output viene eliminato per prevenire eventuali perdite di memoria (memory leaks). */


void CommandContain::writeOutput(ContainOutput *output, float error) const
{
    // Inizializza gli indici per tracciare la posizione corrente negli sketch di query e riferimento
    uint64_t i = output->indexQuery;  // Indice dello sketch di query
    uint64_t j = output->indexRef;    // Indice dello sketch di riferimento
    
    // Itera attraverso tutte le coppie di risultati nel ContainOutput
    for ( uint64_t k = 0; k < output->pairCount && i < output->sketchQuery.getReferenceCount(); k++ )
    {
        // Ottieni un puntatore alla coppia corrente di risultati
        const ContainOutput::PairOutput *pair = &output->pairs[k];
        
        // Verifica se l'errore per questa coppia è inferiore o uguale al valore di errore massimo accettabile
        if ( pair->error <= error )
        {
            // Se l'errore è accettabile, stampa i dettagli della comparazione:
            // - pair->score: Punteggio della comparazione
            // - pair->error: Errore associato a questa coppia
            // - output->sketchRef.getReference(j).name: Nome dello sketch di riferimento
            // - output->sketchQuery.getReference(i).name: Nome dello sketch di query
            cout << pair->score << '\t' << pair->error << '\t' << output->sketchRef.getReference(j).name << '\t' << output->sketchQuery.getReference(i).name << endl;
        }
        
        // Avanza l'indice del riferimento
        j++;
        
        // Se l'indice del riferimento raggiunge il limite, ricomincia e passa al prossimo sketch di query
        if ( j == output->sketchRef.getReferenceCount() )
        {
            j = 0; // Reimposta l'indice del riferimento
            i++;   // Avanza all'indice del prossimo sketch di query
        }
    }
    
    // Dealloca l'oggetto output per evitare fughe di memoria
    delete output;
}


// Questa funzione calcola la stima di containment (inclusione) tra una sequenza query e una sequenza di riferimento.
// La funzione prende come input un puntatore a un oggetto di tipo CommandContain::ContainInput e restituisce un puntatore a un oggetto di tipo CommandContain::ContainOutput.
/**Scopo della Funzione: La funzione contain calcola quanto una sequenza query è contenuta all'interno di una sequenza di riferimento. Il risultato è un "score" che misura questa inclusione, insieme a un errore associato.

Riferimenti agli Sketch: Gli sketch sono rappresentazioni compatte delle sequenze. Questa funzione utilizza gli sketch di una sequenza di riferimento e di una sequenza di query per calcolare il containment.

Ciclo di Comparazione: La funzione esegue un ciclo che confronta ogni coppia di sequenze (query e riferimento) usando la funzione containSketches, che calcola lo score di containment.

Gestione degli Indici: Se si raggiunge la fine delle sequenze di riferimento, il ciclo riparte dalla prima sequenza di riferimento e passa alla sequenza di query successiva.

Restituzione dell'Output: L'output è un oggetto che contiene tutti i risultati dei confronti, che verrà utilizzato per ulteriori elaborazioni o visualizzazioni */
CommandContain::ContainOutput * contain(CommandContain::ContainInput * input)
{
    // Riferimenti costanti agli sketch di riferimento e di query contenuti nell'input.
    // Uno "sketch" è un insieme di min-hashes rappresentativo di una sequenza o di un insieme di sequenze.
    const Sketch & sketchRef = input->sketchRef;
    const Sketch & sketchQuery = input->sketchQuery;
    
    // Creazione di un oggetto di output che conterrà i risultati del calcolo di containment.
    // L'oggetto CommandContain::ContainOutput viene inizializzato con gli sketch di riferimento e di query, 
    // e con gli indici corrispondenti alle sequenze da confrontare.
    CommandContain::ContainOutput * output = new CommandContain::ContainOutput(
        input->sketchRef,     // Sketch di riferimento
        input->sketchQuery,   // Sketch di query
        input->indexRef,      // Indice della sequenza di riferimento corrente
        input->indexQuery,    // Indice della sequenza di query corrente
        input->pairCount      // Numero di coppie (sequenze) da confrontare
    );
    
    // Variabili per tenere traccia degli indici delle sequenze di riferimento e query.
    uint64_t i = input->indexQuery;
    uint64_t j = input->indexRef;
    
    // Ciclo che itera attraverso le coppie di sequenze da confrontare.
    // Il ciclo continua finché ci sono coppie da confrontare e finché ci sono sequenze nella query.
    for ( uint64_t k = 0; k < input->pairCount && i < sketchQuery.getReferenceCount(); k++ )
    {
        // Calcolo del containment (inclusione) tra la sequenza di riferimento e quella di query.
        // La funzione containSketches confronta gli hash delle due sequenze e restituisce un punteggio di containment (score)
        // e un errore associato, che viene salvato nell'output.
        output->pairs[k].score = containSketches(
            sketchRef.getReference(j).hashesSorted,  // Hash della sequenza di riferimento
            sketchQuery.getReference(i).hashesSorted,// Hash della sequenza di query
            output->pairs[k].error                   // Errore del calcolo
        );
        
        // Passa alla sequenza di riferimento successiva.
        j++;
        
        // Se si sono processate tutte le sequenze di riferimento, si resetta l'indice e si passa alla sequenza di query successiva.
        if ( j == sketchRef.getReferenceCount() )
        {
            j = 0;
            i++;
        }
    }
    
    // Restituisce l'output contenente i risultati del calcolo di containment.
    return output;
}


// Questa funzione calcola il livello di "containment" (inclusione) tra due insiemi di hash ordinati, rappresentanti rispettivamente
// una sequenza di riferimento e una sequenza di query. La funzione restituisce un punteggio di containment (come un valore double)
// e imposta un valore di errore associato al calcolo.
double containSketches(const HashList & hashesSortedRef, const HashList & hashesSortedQuery, double & errorToSet)
{
    int common = 0; // Variabile per contare il numero di hash comuni tra riferimento e query.
    
    // Determina la dimensione del denominatore come il minimo tra la dimensione degli hash del riferimento e quelli della query.
    int denom = hashesSortedRef.size() < hashesSortedQuery.size() ?
        hashesSortedRef.size() :
        hashesSortedQuery.size();
    
    // Inizializza gli indici per iterare attraverso gli hash ordinati di riferimento e query.
    int i = 0;
    int j = 0;
    
    // Ciclo per confrontare gli hash tra riferimento e query.
    // Il ciclo continua finché non si raggiunge il denominatore o finché non si esauriscono gli hash del riferimento.
    for ( int steps = 0; steps < denom && i < hashesSortedRef.size(); steps++ )
    {
        // Confronto tra l'hash corrente del riferimento e l'hash corrente della query.
        // Se l'hash del riferimento è minore, si passa all'hash successivo del riferimento.
        if ( hashLessThan(hashesSortedRef.at(i), hashesSortedQuery.at(j), hashesSortedRef.get64()) )
        {
            i++;       // Incrementa l'indice del riferimento.
            steps--;   // Non si considera questo passo come un confronto valido, quindi si decrementa il contatore dei passi.
        }
        // Se l'hash della query è minore, si passa all'hash successivo della query.
        else if ( hashLessThan(hashesSortedQuery.at(j), hashesSortedRef.at(i), hashesSortedRef.get64()) )
        {
            j++;       // Incrementa l'indice della query.
        }
        // Se gli hash sono uguali, significa che c'è un match tra riferimento e query.
        else
        {
            i++;       // Incrementa l'indice del riferimento.
            j++;       // Incrementa l'indice della query.
            common++;  // Incrementa il contatore degli hash comuni.
        }
    }
    
    // Calcola l'errore associato al calcolo del containment, che dipende dal numero di passi effettuati sulla query.
    // L'errore è inversamente proporzionale alla radice quadrata del numero di hash della query considerati.
    errorToSet = 1. / sqrt(j);
    
    // Restituisce il punteggio di containment, che è il rapporto tra il numero di hash comuni e il numero di hash della query considerati.
    return double(common) / j;
}


} // namespace mash
