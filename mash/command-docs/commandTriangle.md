# CommandTriangle

`CommandTriangle` è una classe per l'analisi delle distanze tra sequenze o fingerprint. Questo comando stima una matrice di distanza triangolare inferiore tra tutte le sequenze di input o fingerprint, e la restituisce in formato Phylip rilassato o come lista di edge.

## Descrizione

`CommandTriangle` stima la distanza tra ogni sequenza di input o fingerprint rispetto a tutte le altre sequenze di input. È in grado di gestire file in formato fasta, fastq, gzippati, o file di sketch Mash (.msh) con dimensioni di k-mer corrispondenti. È possibile anche fornire file di nomi di file come input.

Il comando può produrre una matrice di distanza triangolare inferiore in formato Phylip rilassato o una lista di edge, a seconda delle opzioni specificate.

## Opzioni

- `--help`: Mostra l'help e la sintassi del comando.
- `--list, -l`: Indica che i file di input sono elencati in file di testo (un percorso per riga). Questo non influisce sul file di riferimento.
- `--comment, -C`: Usa i campi di commento per i nomi delle sequenze invece degli ID.
- `--edge, -E`: Produce una lista di edge invece della matrice di Phylip. I campi sono [seq1, seq2, dist, p-val, shared-hashes].
- `--pvalue, -v`: Valore massimo di p-value da riportare nella lista di edge. Implica l'uso della lista di edge.
- `--distance, -d`: Distanza massima da riportare nella lista di edge. Implica l'uso della lista di edge.
- `--fingerprint, -fp`: Indica che i file di input sono fingerprint invece di sequenze genomiche.

## Sintassi

```sh
CommandTriangle <seq1> [<seq2>] ...
```

## Esempi

### Calcolo della matrice di distanza

```sh
CommandTriangle seq1.fasta seq2.fasta seq3.fasta
```

### Calcolo della lista di edge con un limite di p-value e distanza

```sh
CommandTriangle --edge --pvalue 0.01 --distance 0.5 seq1.fasta seq2.fasta seq3.fasta
```

### Uso con file di fingerprint

```sh
CommandTriangle --fingerprint --list fingerprints.txt
```

## Funzionamento

1. **Inizializzazione**: Viene creato un oggetto `Sketch` e vengono caricate le sequenze o fingerprint dai file specificati.
2. **Calcolo della distanza**: Viene calcolata la distanza tra ogni coppia di sequenze o fingerprint. Se il flag `--edge` è attivo, viene creata una lista di edge; altrimenti, viene creata una matrice di distanza.
3. **Output**: Il risultato viene stampato in base alle opzioni specificate. Se `--edge` è attivo, la lista di edge include anche i valori di p-value e gli hash condivisi.

## Funzioni

### `run()`

Gestisce l'esecuzione del comando, inclusa l'inizializzazione, il calcolo delle distanze e la produzione dell'output.

### `writeOutput()`

Stampa i risultati dell'analisi. Può formattare l'output come matrice di Phylip o lista di edge.

### `compare()`

Funzione per il calcolo delle distanze tra i riferimenti e le query. Può gestire sia sequenze genomiche che fingerprint.

### `compareFingerprints()`

Confronta due fingerprint e calcola la distanza e il valore di p-value.

## Dipendenze

- **Boost** o **GSL**: Utilizzati per il calcolo del p-value.
- **ThreadPool**: Per l'elaborazione parallela delle distanze.

## Nota

Questo comando è progettato per essere utilizzato in ambienti di analisi di sequenze genomiche e fingerprint, ed è ottimizzato per prestazioni in scenari ad alta parallelizzazione.


# CommandTriangle.h

Questo file header definisce la classe `CommandTriangle`, utilizzata per calcolare una matrice di distanza triangolare inferiore tra sequenze o fingerprint. La classe è una sottoclasse di `Command` e gestisce l'analisi delle distanze tra gli input, con supporto per diverse modalità di confronto e formati di output.

## Classi e Strutture

### `CommandTriangle`

`CommandTriangle` è una sottoclasse di `Command` che implementa un comando per calcolare e stampare una matrice di distanza triangolare inferiore tra le sequenze di input o fingerprint.

#### Metodi Pubblici

- **`CommandTriangle()`**: Costruttore della classe. Inizializza le opzioni e le descrizioni del comando.
- **`int run() const override`**: Metodo principale che esegue il comando. Gestisce l'inizializzazione, l'elaborazione delle distanze e la produzione dell'output.

#### Metodi Privati

- **`void writeOutput(TriangleOutput * output, bool comment, bool edge, double & pValuePeakToSet) const`**: Gestisce la scrittura dei risultati dell'analisi, formattando l'output come matrice di Phylip o lista di edge.

### `TriangleInput`

Struttura utilizzata per memorizzare i dati di input per il confronto.

#### Campi

- **`const Sketch & sketch`**: Riferimento all'oggetto `Sketch` contenente le sequenze o fingerprint da confrontare.
- **`uint64_t index`**: Indice della sequenza di riferimento nel confronto.
- **`const Sketch::Parameters & parameters`**: Parametri di configurazione per l'analisi.
- **`double maxDistance`**: Distanza massima da considerare nei confronti.
- **`double maxPValue`**: P-value massimo da considerare nei confronti.
- **`bool isFingerprint`**: Flag che indica se l'input è un fingerprint invece di una sequenza genomica.

### `TriangleOutput`

Struttura utilizzata per memorizzare i risultati del confronto.

#### Campi

- **`const Sketch & sketch`**: Riferimento all'oggetto `Sketch` utilizzato per il confronto.
- **`uint64_t index`**: Indice della sequenza di riferimento per la quale sono calcolate le distanze.
- **`CommandDistance::CompareOutput::PairOutput * pairs`**: Array di risultati del confronto per ogni coppia di sequenze.

#### Metodi

- **`TriangleOutput(const Sketch & sketchNew, uint64_t indexNew)`**: Costruttore che inizializza l'oggetto e alloca l'array di risultati.
- **`~TriangleOutput()`**: Distruttore che libera l'array di risultati.

## Funzioni

### `CommandTriangle::compare()`

Funzione esterna che esegue il confronto tra due sequenze o fingerprint e restituisce un oggetto `TriangleOutput` contenente i risultati.

#### Parametri

- **`TriangleInput * input`**: Puntatore alla struttura `TriangleInput` contenente i dati di input per il confronto.

#### Ritorna

- **`TriangleOutput *`**: Puntatore a un oggetto `TriangleOutput` contenente i risultati del confronto.

### `compareFingerprints()`

Funzione esterna che confronta due fingerprint e calcola la distanza e il p-value.

#### Parametri

- **`CommandDistance::CompareOutput::PairOutput * pair`**: Puntatore alla struttura che memorizza il risultato del confronto.
- **`const Sketch::Reference & ref1`**: Primo fingerprint di riferimento.
- **`const Sketch::Reference & ref2`**: Secondo fingerprint di riferimento.
- **`uint64_t sketchSize`**: Dimensione dello sketch.
- **`double maxDistance`**: Distanza massima da considerare.
- **`double maxPValue`**: P-value massimo da considerare.

## Include

- **`Command.h`**: Include la classe base `Command`.
- **`CommandDistance.h`**: Include la definizione della classe `CommandDistance` e i risultati del confronto.
- **`Sketch.h`**: Include la definizione della classe `Sketch` e dei riferimenti.

## Note

Il file `CommandTriangle.h` è una parte essenziale per l'analisi e il confronto delle sequenze o fingerprint. Definisce le strutture necessarie per gestire l'input e l'output del confronto, e include le dichiarazioni delle funzioni utilizzate per il calcolo delle distanze.
