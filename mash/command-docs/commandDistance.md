# CommandDistance.cpp

## Descrizione

Il file `CommandDistance.cpp` implementa la classe `CommandDistance` definita in `CommandDistance.h`. Questa classe è utilizzata per confrontare sketch genomici e calcolare le distanze tra di essi, utilizzando vari algoritmi e tecniche per gestire input e output, e parallelizzare il processo tramite un thread pool.

## Implementazione della Classe

### Costruttore

#### `CommandDistance::CommandDistance()`

Il costruttore della classe `CommandDistance` inizializza il nome, la sintesi e la descrizione del comando. Definisce anche le opzioni della riga di comando disponibili, come `--list`, `--table`, `--pvalue`, `--distance`, `--comment`, e `--fingerprint`, e verifica che le opzioni di sketch siano utilizzabili.

### Metodo Principale

#### `int CommandDistance::run() const`

Il metodo `run` gestisce il flusso principale del comando:

1. **Verifica degli Argomenti**: Controlla se gli argomenti sono sufficienti o se è richiesta l'opzione di aiuto. Se sì, stampa l'aiuto e termina l'esecuzione.
2. **Gestione delle Opzioni**: Estrae le opzioni della riga di comando, come il numero di thread, le opzioni per output in formato tabella, valori di p e distanza massimi, e se si utilizza il formato fingerprint.
3. **Inizializzazione del Reference Sketch**: Carica lo sketch di riferimento, gestisce gli errori se le opzioni di sketch sono incoerenti e configura i parametri in base al tipo di file di riferimento.
4. **Output Tabellare**: Se l'opzione `--table` è attiva, stampa le intestazioni delle colonne.
5. **Thread Pool e Parallelizzazione**: Utilizza un thread pool per eseguire i confronti tra sketch in parallelo. Suddivide i file di query in base alle opzioni fornite e crea gli sketch di query.
6. **Scrittura dell'Output**: Gestisce la scrittura dei risultati utilizzando il metodo `writeOutput`.

### Metodi di Supporto

#### `void CommandDistance::writeOutput(CompareOutput * output, bool table, bool comment) const`

Il metodo `writeOutput` stampa i risultati del confronto. Se l'opzione `--table` è attiva, stampa i risultati in formato tabella; altrimenti, stampa i risultati con dettagli come nome e commento delle sequenze di riferimento e query.

#### `CommandDistance::CompareOutput * compare(CommandDistance::CompareInput * input)`

Il metodo `compare` esegue il confronto tra due sketch e restituisce i risultati in una struttura `CompareOutput`. Utilizza `compareSketches` per calcolare i dettagli del confronto e gestisce eccezioni nel caso di errori.

#### `void compareSketches(CommandDistance::CompareOutput::PairOutput * output, const Sketch::Reference & refRef, const Sketch::Reference & refQry, uint64_t sketchSize, int kmerSize, double kmerSpace, double maxDistance, double maxPValue)`

Il metodo `compareSketches` calcola la distanza tra due sketch e il valore di p associato. Utilizza l'algoritmo di Jaccard per calcolare la distanza e gestisce casi speciali per evitare valori negativi o infiniti.

#### `double pValue(uint64_t x, uint64_t lengthRef, uint64_t lengthQuery, double kmerSpace, uint64_t sketchSize)`

Il metodo `pValue` calcola il valore di p utilizzando una distribuzione binomiale. A seconda della definizione di `USE_BOOST`, utilizza la libreria Boost o GSL per il calcolo.

## Utilizzo

La classe `CommandDistance` è progettata per confrontare sketch genomici, fornendo una valutazione della distanza e dei valori di p tra sequenze di query e di riferimento. È particolarmente utile per l'analisi comparativa di sequenze genomiche e può gestire vari formati di input.

## Dipendenze

Il file include le seguenti dipendenze:

- `CommandDistance.h`: Definisce la classe `CommandDistance`.
- `Sketch.h`: Definisce la classe `Sketch` e le sue strutture.
- `ThreadPool.h`: Fornisce un'implementazione di thread pool per la parallelizzazione.
- `zlib.h`: Per la gestione della compressione/decompressione.
- `boost/math/distributions/binomial.hpp` o `gsl/gsl_cdf.h`: Per il calcolo del valore di p, a seconda delle opzioni di compilazione.

## Note

- Le opzioni `--fingerprint` sono state aggiunte per supportare i file di fingerprint.
- Il metodo `compareSketches` gestisce casi speciali per evitare risultati anomali.

# CommandDistance.h

## Descrizione

Il file `CommandDistance.h` definisce la classe `CommandDistance`, un componente essenziale del software Mash utilizzato per confrontare due sketch genomici e calcolare le distanze tra di essi. La classe offre strutture dati e metodi per gestire l'input e l'output del confronto, nonché per calcolare i valori di p associati alle distanze stimate.

### Strutture Dati

#### `CommandDistance::CompareInput`

Questa struttura contiene i dati necessari per eseguire un confronto tra due sketch genomici:

- **`sketchRef`**: Il primo sketch da confrontare.
- **`sketchQuery`**: Il secondo sketch da confrontare.
- **`indexRef`**: Indice del primo sketch.
- **`indexQuery`**: Indice del secondo sketch.
- **`pairCount`**: Numero di coppie di k-mer da confrontare.
- **`parameters`**: Parametri utilizzati nel confronto.
- **`maxDistance`**: Distanza massima da considerare.
- **`maxPValue`**: Valore di p massimo da considerare.

#### `CommandDistance::CompareOutput`

Questa struttura memorizza i risultati del confronto tra due sketch:

- **`sketchRef`**: Il primo sketch confrontato.
- **`sketchQuery`**: Il secondo sketch confrontato.
- **`indexRef`**: Indice del primo sketch.
- **`indexQuery`**: Indice del secondo sketch.
- **`pairCount`**: Numero di coppie di k-mer confrontate.
- **`pairs`**: Array di `PairOutput` contenente i risultati del confronto.

##### `CommandDistance::CompareOutput::PairOutput`

Struttura per memorizzare i risultati di una singola coppia di k-mer:

- **`numer`**: Numeratore della distanza.
- **`denom`**: Denominatore della distanza.
- **`distance`**: Distanza calcolata.
- **`pValue`**: Valore di p associato alla distanza.
- **`pass`**: Booleano che indica se il confronto è passato o meno.

## Metodi

### `CommandDistance::CommandDistance()`

Costruttore della classe `CommandDistance`.

### `int CommandDistance::run() const`

Metodo per eseguire il comando di confronto tra sketch.

### `void CommandDistance::writeOutput(CompareOutput * output, bool table, bool comment) const`

Metodo per scrivere l'output del confronto in un formato specificato (tabella o commento).

### `CommandDistance::CompareOutput * compare(CommandDistance::CompareInput * input)`

Funzione per eseguire il confronto tra due sketch e restituire i risultati.

### `void compareSketches(CommandDistance::CompareOutput::PairOutput * output, const Sketch::Reference & refRef, const Sketch::Reference & refQry, uint64_t sketchSize, int kmerSize, double kmerSpace, double maxDistance, double maxPValue)`

Funzione per calcolare i valori di distanza e p-value per coppie di k-mer tra due sketch.

### `double pValue(uint64_t x, uint64_t lengthRef, uint64_t lengthQuery, double kmerSpace, uint64_t sketchSize)`

Funzione per calcolare il valore di p basato sui parametri di confronto.

## Utilizzo

La classe `CommandDistance` è progettata per essere utilizzata come parte del software Mash per il confronto tra sketch genomici. Può essere usata per eseguire confronti tra diversi set di dati e calcolare distanze e valori di p associati. Questo è particolarmente utile per l'analisi comparativa di sequenze genomiche.

## Dipendenze

Il file include le seguenti dipendenze:

- `Command.h`: Definisce la classe base `Command`.
- `Sketch.h`: Definisce la classe `Sketch` e le relative strutture e metodi utilizzati per la gestione degli sketch genomici.

Assicurati di includere anche queste dipendenze nel tuo progetto per utilizzare correttamente `CommandDistance.h`.
