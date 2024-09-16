# CommandPaste

## Descrizione Generale

## File `CommandPaste.cpp`

---

### Costruttore `CommandPaste::CommandPaste`

#### Descrizione

Il costruttore della classe `CommandPaste` inizializza un'istanza del comando "paste". Questo comando è progettato per creare un singolo file di sketch a partire da più file di sketch. Il costruttore configura i dettagli del comando, inclusi il nome, la descrizione e le opzioni disponibili.

#### Inizializzazione dei Membri

- **`name = "paste";`**  
  Imposta il nome del comando su "paste". Questo valore viene utilizzato per identificare il comando quando viene eseguito o visualizzato.

- **`summary = "Create a single sketch file from multiple sketch files.";`**  
  Fornisce una breve descrizione del comando, che riassume la sua funzione principale: creare un singolo file di sketch combinando più file di sketch.

- **`description = "Create a single sketch file from multiple sketch files.";`**  
  Fornisce una descrizione dettagliata del comando. Specifica che il comando crea un file di sketch unificato partendo da più file di sketch.

- **`argumentString = "<out_prefix> <sketch> [<sketch>] ...";`**  
  Definisce la sintassi degli argomenti che il comando accetta. In questo caso, `<out_prefix>` rappresenta il prefisso del file di output, mentre `<sketch>` rappresenta uno o più file di sketch da combinare.

#### Opzioni del Comando

- **`useOption("help");`**  
  Aggiunge un'opzione di aiuto al comando. Quando viene specificata l'opzione "help", viene visualizzata una guida sull'uso del comando.

- **`addOption("list", Option(Option::Boolean, "l", "", "Input files are lists of file names.", ""));`**  
  Aggiunge un'opzione denominata "list" al comando. Se l'opzione è specificata, il comando interpreta i file di input come liste di nomi di file. La sintassi dell'opzione è `-l`.

- **`addOption("fingerPrint", Option(Option::Boolean, "fp", "", "Insert fingerprint files are lists of file names.", ""));`**  
  Aggiunge un'opzione denominata "fingerPrint" al comando. Se l'opzione è specificata, il comando accetta file di tipo fingerprint in formato .txt. La sintassi dell'opzione è `-fp`.

- **`addOption("output", Option(Option::Boolean, "o", "", "Insert -o to indicate the name and path for the output file.\n Take this option as the last one after -fp or -l ", ""));`**  
  Aggiunge un'opzione denominata "output" al comando. Se l'opzione è specificata, consente di indicare il nome e il percorso del file di output. Questa opzione deve essere specificata dopo le opzioni `-fp` o `-l`. La sintassi dell'opzione è `-o`.

---

### Funzione `fileExists`

#### Descrizione

La funzione `fileExists` verifica l'esistenza di un file nel filesystem. Essa restituisce un valore booleano che indica se il file specificato è presente o meno.

#### Parametri

- **`filename`** (`const std::string&`): Il percorso del file da controllare. Deve essere una stringa che rappresenta il nome e il percorso del file nel filesystem.

#### Valore di Ritorno

- **`bool`**: Restituisce `true` se il file esiste, altrimenti `false`.

#### Comportamento

- Utilizza la funzione `access` della libreria POSIX per controllare la presenza del file.
- La costante `F_OK` viene utilizzata per verificare l'esistenza del file, senza controllare i permessi di lettura, scrittura o esecuzione.
- Se `access` restituisce un valore diverso da `-1`, significa che il file esiste e la funzione restituisce `true`. Altrimenti, restituisce `false`.

### Funzione `CommandPaste::run`

#### Descrizione

La funzione `run` è il punto di ingresso principale per l'esecuzione del comando `paste`. Questo comando combina più file di sketch in un singolo file di output. La funzione gestisce le opzioni di input, verifica la validità dei file, e scrive il risultato finale in un file di output.

#### Funzionalità

1. **Gestione delle Opzioni**:
   - Controlla se le opzioni `output`, `list`, e `fingerPrint` sono attive e gestisce i conflitti tra di esse.
   - Se entrambe `list` e `fingerPrint` sono attive, emette un errore poiché queste opzioni sono incompatibili.

2. **Verifica degli Argomenti**:
   - Se il numero di argomenti è insufficiente o se l'opzione `help` è attiva, stampa le istruzioni di utilizzo e termina l'esecuzione.

3. **Preparazione dei File di Input**:
   - Popola un vettore di nomi di file (`files`) basato sugli argomenti passati e sull'opzione `output`.
   - Se l'opzione `output` è attiva, il file di output viene impostato come ultimo argomento. Altrimenti, viene impostato come primo argomento.
   - Gestisce le opzioni `list` e `fingerPrint` per determinare come vengono trattati i file di input.

4. **Verifica della Validità dei File**:
   - Controlla che ogni file abbia un suffisso valido (`.txt` o `.msh` per `fingerPrint` e `.sketch` per altri casi).
   - Se un file `.txt` è trovato, verifica l'esistenza del corrispondente file `.msh`.
   - Se un file `.msh` è trovato, verifica l'esistenza del corrispondente file `.txt`.
   - Aggiunge i file validi a un vettore separato (`filesGood`) e stampa errori se i file necessari non esistono.

5. **Inizializzazione e Scrittura**:
   - Inizializza un oggetto `Sketch` utilizzando i file validi e i parametri di configurazione.
   - Aggiunge un suffisso `.sketch` al nome del file di output se non è già presente.
   - Verifica se il file di output esiste già e, se sì, stampa un errore e termina l'esecuzione.
   - Scrive l'oggetto `Sketch` nel file di output specificato.

#### Parametri

- **Nessun parametro diretto**: La funzione utilizza i membri della classe e le opzioni di comando per determinare il comportamento.

#### Valore di Ritorno

- **`int`**: Restituisce `0` per indicare il successo. Restituisce `1` in caso di errore, come file mancanti o conflitti di opzioni.

#### Comportamento di Errore

- Stampa messaggi di errore su `cerr` in caso di opzioni incompatibili, file non trovati, o file di output già esistente.
- Termina il programma in caso di errori critici utilizzando `exit(1)`.

## File `CommandPaste.h`

### Descrizione Generale

Il file `CommandPaste.h` definisce la classe `CommandPaste`, che estende la classe base `Command`. Questa classe è progettata per gestire un comando specifico all'interno di un sistema che opera su sketch e comandi associati. Il file include anche le dichiarazioni necessarie per l'inclusione e l'utilizzo della classe `CommandPaste`.

### Macro di Protezione

- **`#ifndef INCLUDED_CommandPaste`**  
  **`#define INCLUDED_CommandPaste`**  
  **`#endif`**

  Le macro di protezione garantiscono che il file di intestazione (`CommandPaste.h`) venga incluso solo una volta durante la compilazione. Questo previene errori di ridondanza e conflitti di inclusione.

### Inclusioni

- **`#include "Command.h"`**

  Includa il file di intestazione `Command.h`, che contiene la dichiarazione della classe base `Command`. La classe `CommandPaste` estende questa classe base, quindi è necessario includere il suo header.

- **`#include "Sketch.h"`**

  Includa il file di intestazione `Sketch.h`, che probabilmente contiene definizioni e dichiarazioni relative agli sketch. Anche se non viene utilizzato direttamente nel file `CommandPaste.h`, potrebbe essere necessario per la futura estensione della classe `CommandPaste`.

### Namespace

- **`namespace mash { ... }`**

  Tutte le dichiarazioni sono racchiuse all'interno del namespace `mash`. Questo spazio dei nomi viene utilizzato per evitare conflitti di nome e raggruppare le classi e le funzioni correlate.

### Classe `CommandPaste`

La classe `CommandPaste` estende la classe base `Command` e implementa funzionalità specifiche per un comando di tipo "paste".

#### Membri Pubblici

- **`CommandPaste();`**

  Costruttore della classe `CommandPaste`. Inizializza una nuova istanza della classe `CommandPaste`. Dettagli specifici su cosa fa il costruttore non sono forniti nel file, ma è probabile che configuri lo stato iniziale dell'oggetto.

- **`int run() const;`**

  Metodo `run` che esegue il comando di paste. Questo metodo è un'override del metodo `run` della classe base `Command`. Restituisce un intero che indica il risultato dell'esecuzione del comando. Dettagli su come viene implementata l'operazione di paste non sono specificati nel file, ma il metodo è previsto per gestire la logica associata al comando "paste".

### Macro di Protezione

- **`#ifndef INCLUDED_CommandPaste`**  
  **`#define INCLUDED_CommandPaste`**  
  **`#endif`**

  Le macro di protezione sono utilizzate per evitare che il file di intestazione venga incluso più di una volta durante la compilazione. Questo garantisce che le dichiarazioni siano processate correttamente senza errori di inclusione multipla.

---
