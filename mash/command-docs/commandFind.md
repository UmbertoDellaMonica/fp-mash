# CommandFind.cpp

Questo file contiene l'implementazione della classe `CommandFind` e delle funzioni associate per la ricerca di sequenze in uno sketch. La classe `CommandFind` estende la classe base `Command` e gestisce la ricerca di sequenze nel contesto di un insieme di dati rappresentato come sketch.

## Implementazione della Classe

### `CommandFind`

#### Costruttore

- **`CommandFind()`**
  - Inizializza il comando `CommandFind` e configura le opzioni di linea di comando, come il threshold, il numero di migliori risultati da riportare, e la gestione delle corrispondenze con se stessi.

#### Metodi Pubblici

- **`int run() const`**
  - Gestisce l'esecuzione del comando. Questo metodo gestisce la lettura dei file di riferimento e di query, crea gli sketch se necessario, e utilizza un pool di thread per eseguire la ricerca. Alla fine, raccoglie e scrive i risultati della ricerca.

#### Metodi Privati

- **`void writeOutput(const Sketch & sketch, FindOutput * output) const`**
  - Scrive i risultati della ricerca. Ordina i risultati per punteggio e li stampa.

### Strutture Dati

#### `FindInput`

- **Costruttore**
  - Inizializza i parametri necessari per la ricerca, inclusi lo sketch, la sequenza, la lunghezza della sequenza, la soglia, il numero di migliori risultati e se sono ammesse corrispondenze con se stessi.
- **Distruttore**
  - Libera la memoria allocata per la sequenza.

#### `FindOutput`

- **Struttura `Hit`**
  - Rappresenta una singola corrispondenza trovata. Contiene informazioni come l'indice di riferimento, l'inizio e la fine della corrispondenza, il filamento e il punteggio.

### Funzioni Non Membri

- **`CommandFind::FindOutput * find(CommandFind::FindInput * data)`**
  - Esegue la ricerca e restituisce i risultati. Gestisce sia la ricerca per il filamento diretto che per il filamento complementare.

- **`void findPerStrand(const CommandFind::FindInput * input, CommandFind::FindOutput * output, bool minusStrand)`**
  - Esegue la ricerca per un singolo filamento. Calcola i min-hashes della sequenza e confronta con gli sketch di riferimento.

- **`bool operator<(const CommandFind::FindOutput::Hit & a, const CommandFind::FindOutput::Hit & b)`**
  - Operatore di confronto per ordinare le corrispondenze per punteggio nella coda di priorità.

## Inclusioni

- **`CommandFind.h`**: Include la dichiarazione della classe `CommandFind` e delle sue strutture dati.
- **`Sketch.h`**: Include la dichiarazione della classe `Sketch`, utilizzata da `CommandFind`.
- **`zlib.h`**: Fornisce funzioni per la gestione di file compressi.
- **`kseq.h`**: Fornisce la lettura delle sequenze da file FASTA e FASTQ.
- **`robin_hood.h`**: Fornisce implementazioni di hash table e set.
- **`ThreadPool.h`**: Fornisce la gestione del pool di thread per l'elaborazione parallela.
- **`sketchParameterSetup.h`**: Fornisce la configurazione dei parametri per lo sketch.

## Note

Il file `CommandFind.cpp` implementa la logica per la ricerca di sequenze in uno sketch. Gestisce la lettura dei file di riferimento e di query, la creazione di sketch se necessario, e utilizza un pool di thread per eseguire la ricerca in parallelo. I risultati vengono ordinati e stampati sulla console.

# CommandFind.h

Questo file contiene la dichiarazione della classe `CommandFind`, una sottoclasse di `Command` progettata per cercare sequenze all'interno di sketch. Questa classe gestisce la ricerca di sequenze nel contesto di un insieme di dati rappresentato come sketch.

## Descrizione della Classe

### `CommandFind`

`CommandFind` estende la classe base `Command` e implementa la logica per cercare sequenze in uno sketch.

#### Strutture Dati

- **`FindInput`**: Struttura che memorizza i dati di input necessari per la ricerca.
  - **`const Sketch & sketch`**: Riferimento allo sketch in cui eseguire la ricerca.
  - **`std::string seqId`**: Identificativo della sequenza da cercare.
  - **`char * seq`**: Sequenza da cercare.
  - **`uint32_t length`**: Lunghezza della sequenza da cercare.
  - **`float threshold`**: Soglia di punteggio per considerare una corrispondenza.
  - **`int best`**: Numero di migliori corrispondenze da restituire.
  - **`bool selfMatches`**: Flag che indica se sono ammesse corrispondenze con se stessi.

- **`FindOutput`**: Struttura che contiene i risultati della ricerca.
  - **`struct Hit`**: Rappresenta una singola corrispondenza trovata.
    - **`unsigned int ref`**: Indice di riferimento della sequenza trovata.
    - **`unsigned int start`**: Indice di inizio della corrispondenza.
    - **`unsigned int end`**: Indice di fine della corrispondenza.
    - **`bool minusStrand`**: Indica se la corrispondenza è sul filamento complementare.
    - **`float score`**: Punteggio della corrispondenza.
  - **`std::string seqId`**: Identificativo della sequenza cercata.
  - **`std::priority_queue<Hit>`**: Coda di priorità contenente le corrispondenze ordinate per punteggio.

#### Costruttori e Metodi Pubblici

- **`CommandFind()`**: Costruttore che inizializza il comando `CommandFind`.
- **`int run() const`**: Metodo che esegue il comando `CommandFind`. Questo metodo è un override del metodo `run` della classe base `Command` e gestisce la ricerca della sequenza all'interno dello sketch.

#### Metodi Privati

- **`void writeOutput(const Sketch & sketch, FindOutput * output) const`**: Metodo che gestisce la scrittura dei risultati della ricerca.

#### Funzioni Non Membri

- **`CommandFind::FindOutput * find(CommandFind::FindInput * data)`**: Funzione che esegue la ricerca e restituisce i risultati.
- **`void findPerStrand(const CommandFind::FindInput * input, CommandFind::FindOutput * output, bool minusStrand)`**: Funzione che esegue la ricerca per un singolo filamento (diretta o complementare).
- **`bool operator<(const CommandFind::FindOutput::Hit & a, const CommandFind::FindOutput::Hit & b)`**: Operatore di confronto per ordinare le corrispondenze per punteggio.

## Include

- **`Command.h`**: Include la dichiarazione della classe base `Command`, dalla quale `CommandFind` eredita.
- **`Sketch.h`**: Include la dichiarazione della classe `Sketch`, utilizzata da `CommandFind`.

## Note

Il file `CommandFind.h` definisce la struttura della classe `CommandFind` e le sue strutture dati associate. Questo file è essenziale per definire le interfacce e le strutture necessarie per implementare la ricerca di sequenze nel contesto degli sketch.
