# CommandSketch.cpp

Questo file implementa la classe `CommandSketch`, che crea rappresentazioni ridotte (sketches) di sequenze genomiche o fingerprint per operazioni rapide. Utilizza l'algoritmo di min-hashing per ridurre le sequenze a sketch che possono essere usati per stime di distanza veloci. Le entrate possono essere file FASTA, FASTQ, o file di fingerprint. L'output è un file di sketch (.msh).

## Descrizione della Classe

### `CommandSketch`

`CommandSketch` è una sottoclasse di `Command` che gestisce la creazione di sketch da file di sequenze o fingerprint.

#### Costruttore

- **`CommandSketch()`**: Costruttore che inizializza le opzioni e le descrizioni del comando.

#### Metodi Pubblici

- **`int run() const`**: Metodo principale che esegue il comando. Gestisce la lettura dei file di input, la creazione degli sketch, e la scrittura del file di output.

## Opzioni e Argomenti

- **`list` (`-l`)**: Indica che il file di input è una lista di percorsi a file di sequenze. Ogni linea nel file specifica un percorso.
- **`prefix` (`-o`)**: Prefix del file di output. Se non specificato, viene usato il primo file di input con l'estensione '.msh' aggiunta.
- **`id` (`-I`)**: Campo ID per lo sketch delle letture, anziché il primo ID della sequenza.
- **`comment` (`-C`)**: Commento per lo sketch delle letture, anziché il primo commento della sequenza.
- **`counts` (`-M`)**: Se attivo, memorizza la molteplicità di ogni k-mer in ogni sketch.
- **`fingerprint` (`-fp`)**: Indica che i file di input sono fingerprint anziché sequenze genomiche.

## Metodi Privati

- **`void print() const`**: Stampa l'uso del comando e le opzioni disponibili. Utilizzato quando non ci sono argomenti o è richiesta l'opzione di aiuto.

## Funzioni

### `CommandSketch::run()`

Il metodo `run()` gestisce il flusso principale del comando. Controlla le opzioni e gli argomenti, inizializza gli sketch, e scrive l'output in un file.

1. **Gestione degli Argomenti**: Se non ci sono argomenti o è richiesta l'opzione di aiuto, stampa l'uso del comando.
2. **Opzioni di Configurazione**: Configura i parametri dello sketch come la memorizzazione dei conteggi e l'inizializzazione della classe `Sketch`.
3. **Lettura dei File di Input**: Gestisce file di input, inclusi file di lista se specificato.
4. **Creazione degli Sketch**: Inizializza gli sketch da file, letture o fingerprint.
5. **Scrittura del File di Output**: Determina il nome del file di output e scrive gli sketch nel formato Cap'n Proto.

## Include

- **`Command.h`**: Include la classe base `Command`.
- **`Sketch.h`**: Include la definizione della classe `Sketch` e dei parametri.
- **`sketchParameterSetup.h`**: Include la funzione `sketchParameterSetup` per configurare i parametri dello sketch.
- **`iostream`**: Per la gestione dell'input e output standard.

## Note

Il file `CommandSketch.cpp` è una parte essenziale del sistema per la creazione e gestione degli sketch. Gestisce la lettura dei file di input, la creazione degli sketch utilizzando le opzioni specificate, e la scrittura dell'output in formato Cap'n Proto.

# CommandSketch.h

Questo file contiene la dichiarazione della classe `CommandSketch`, che è una sottoclasse di `Command` utilizzata per creare rappresentazioni ridotte (sketches) di sequenze genomiche o fingerprint. Questi sketch sono usati per operazioni rapide di stima delle distanze.

## Descrizione della Classe

### `CommandSketch`

`CommandSketch` estende la classe base `Command` e implementa la logica per la creazione di sketch.

#### Costruttore

- **`CommandSketch()`**: Costruttore che inizializza il comando `CommandSketch`.

#### Metodi Pubblici

- **`int run() const`**: Metodo che esegue il comando `CommandSketch`. Questo metodo è un override del metodo `run` della classe base `Command` e gestisce la creazione degli sketch a partire dai file di input.

## Include

- **`Command.h`**: Include la dichiarazione della classe base `Command`, dalla quale `CommandSketch` eredita.

## Note

Il file `CommandSketch.h` definisce la struttura della classe `CommandSketch` e il suo metodo principale `run()`, che gestisce l'esecuzione del comando di creazione degli sketch. Questo file è essenziale per definire le interfacce e le funzioni che saranno implementate nel file `CommandSketch.cpp`.
