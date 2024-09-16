// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#ifndef INCLUDED_CommandContain
#define INCLUDED_CommandContain

#include "Command.h"
#include "Sketch.h"

namespace mash {

class CommandContain : public Command
{
public:
    
    


/**Scopo della Struttura: La struttura ContainInput è utilizzata per incapsulare i dati necessari per eseguire il calcolo del containment (cioè la misura della sovrapposizione) tra due set di sketch: uno di riferimento (sketchRef) e uno di query (sketchQuery). Questo è tipicamente utilizzato all'interno di un sistema che elabora grandi quantità di dati biologici, ad esempio sequenze genomiche.

Membri della Struttura:

sketchRef: Questo è un riferimento costante allo sketch di riferimento, che rappresenta il dataset di base contro cui vengono confrontate le sequenze query.
sketchQuery: Questo è un riferimento costante allo sketch di query, che rappresenta il dataset che viene confrontato con il riferimento per determinare la misura di containment.
indexRef e indexQuery: Questi due indici rappresentano le posizioni correnti nei rispettivi sketch (riferimento e query) da cui iniziare il confronto. Sono utili quando si eseguono confronti parziali o iterati.
pairCount: Questo membro rappresenta il numero di coppie di sketch che devono essere confrontate. È utile per gestire il lavoro in parallelo, dividendo il compito tra più thread.
nameRef: Questo è una stringa che memorizza il nome del riferimento, che può essere usata per identificare il riferimento nel contesto del confronto. Tuttavia, in questa struttura specifica, il membro sembra non essere utilizzato direttamente nel costruttore o altrove.
parameters: Questo è un riferimento costante alla struttura dei parametri di sketching, che include informazioni come la dimensione del k-mer, la soglia di errore, e altri dettagli necessari per eseguire il confronto corretto. */

// La struttura ContainInput viene utilizzata per incapsulare i dati di input necessari 
// per il calcolo del containment tra due sketch (riferimento e query).
struct ContainInput
{
    // Costruttore della struttura ContainInput. Inizializza i membri con i valori passati come argomenti.
    ContainInput(const Sketch & sketchRefNew, const Sketch & sketchQueryNew, uint64_t indexRefNew, uint64_t indexQueryNew, uint64_t pairCountNew, const Sketch::Parameters & parametersNew)
        :
        // Inizializza i membri con i valori passati al costruttore
        sketchRef(sketchRefNew),
        sketchQuery(sketchQueryNew),
        indexRef(indexRefNew),
        indexQuery(indexQueryNew),
        pairCount(pairCountNew),
        parameters(parametersNew)
        {}

    // Riferimento costante allo sketch di riferimento (riferimento al dataset su cui si basa il confronto).
    const Sketch & sketchRef;

    // Riferimento costante allo sketch di query (dataset che viene confrontato con il riferimento).
    const Sketch & sketchQuery;

    // Indice che rappresenta la posizione corrente nello sketch di riferimento.
    uint64_t indexRef;

    // Indice che rappresenta la posizione corrente nello sketch di query.
    uint64_t indexQuery;

    // Numero di coppie di sketch da confrontare (determina il numero di confronti da fare).
    uint64_t pairCount;
    
    // Nome del riferimento (utilizzato per identificare il riferimento nel contesto del confronto).
    std::string nameRef;

    // Riferimento costante ai parametri utilizzati per lo sketching (ad esempio, k-mer size, soglia di errore, ecc.).
    const Sketch::Parameters & parameters;
};


    
// La struttura ContainOutput viene utilizzata per raccogliere e gestire i risultati
// del calcolo del containment tra due sketch (riferimento e query).
/**Scopo della Struttura: La struttura ContainOutput serve a raccogliere i risultati del calcolo del containment tra due insiemi di sketch, uno di riferimento (sketchRef) e uno di query (sketchQuery). Ogni confronto tra una coppia di sketch produce un risultato (punteggio e errore) che viene memorizzato nella struttura interna PairOutput.

Membri della Struttura:

PairOutput: Questa è una struttura interna che contiene il risultato del confronto tra due sketch:
score: Rappresenta il punteggio di containment, cioè la frazione di min-hashes che sono comuni tra il reference e il query.
error: Indica l'errore associato al punteggio, calcolato in base a un modello statistico.
sketchRef: Riferimento costante allo sketch di riferimento utilizzato nel confronto. Serve a evitare la copia dei dati, migliorando l'efficienza.
sketchQuery: Riferimento costante allo sketch di query che viene confrontato con il riferimento.
indexRef e indexQuery: Questi due indici indicano le posizioni correnti nei rispettivi sketch, che aiutano a gestire il processo di confronto in modo iterativo.
pairCount: Indica il numero di coppie di sketch da confrontare, ovvero la dimensione dell'array pairs.
pairs: Puntatore a un array dinamico di PairOutput, che memorizza i risultati di ciascun confronto.
Costruttore:

Il costruttore inizializza i membri della struttura con i valori forniti e alloca dinamicamente un array di PairOutput della dimensione pairCount, che contiene i risultati dei confronti.
Distruttore:

Il distruttore rilascia la memoria allocata per l'array pairs, prevenendo fughe di memoria (memory leaks). */
struct ContainOutput
{
    // Costruttore della struttura ContainOutput. Inizializza i membri con i valori passati come argomenti.
    // Inoltre, alloca dinamicamente un array di PairOutput, ciascuno dei quali conterrà un risultato del confronto.
    ContainOutput(const Sketch & sketchRefNew, const Sketch & sketchQueryNew, uint64_t indexRefNew, uint64_t indexQueryNew, uint64_t pairCountNew)
        :
        sketchRef(sketchRefNew),
        sketchQuery(sketchQueryNew),
        indexRef(indexRefNew),
        indexQuery(indexQueryNew),
        pairCount(pairCountNew)
    {
        // Alloca dinamicamente un array di PairOutput con dimensione pari al numero di coppie da confrontare
        pairs = new PairOutput[pairCount];
    }
    
    // Distruttore della struttura ContainOutput.
    // Libera la memoria allocata dinamicamente per l'array di PairOutput.
    ~ContainOutput()
    {
        delete [] pairs;
    }
    
    // Struttura interna PairOutput. Utilizzata per memorizzare il risultato del confronto di una coppia di sketch.
    struct PairOutput
    {
        // Punteggio del confronto tra due sketch. 
        // Rappresenta la frazione di min-hashes condivisi tra i due sketch.
        double score;

        // Errore associato al calcolo del punteggio, solitamente basato su un modello statistico.
        double error;
    };

    // Riferimento costante allo sketch di riferimento (riferimento al dataset su cui si basa il confronto).
    const Sketch & sketchRef;

    // Riferimento costante allo sketch di query (dataset che viene confrontato con il riferimento).
    const Sketch & sketchQuery;

    // Indice che rappresenta la posizione corrente nello sketch di riferimento.
    uint64_t indexRef;

    // Indice che rappresenta la posizione corrente nello sketch di query.
    uint64_t indexQuery;

    // Numero di coppie di sketch confrontate (determina la dimensione dell'array pairs).
    uint64_t pairCount;

    // Puntatore a un array di PairOutput, ciascun elemento rappresenta il risultato di un confronto.
    PairOutput * pairs;
};

    
    CommandContain();
    
    int run() const; // override
    
private:
    
    void writeOutput(ContainOutput * output, float error) const;
};

CommandContain::ContainOutput * contain(CommandContain::ContainInput * data);

double containSketches(const HashList & hashesSortedRef, const HashList & hashesSortedQuery, double & errorToSet);

} // namespace mash

#endif
