# Documentazione di CommandInfo

## Descrizione Generale

La classe `CommandInfo` fa parte di un software sviluppato per lavorare con file di sketch. Questa classe fornisce il comando `info`, che consente di visualizzare informazioni sui file di sketch, come dettagli dell'intestazione, conteggi di hash, output tabellare, e l'esportazione in formato JSON.

## File `CommandInfo.cpp`

---

### Costruttore `CommandInfo::CommandInfo`

#### Descrizione

Il costruttore della classe `CommandInfo` inizializza un'istanza del comando `info`. Questo comando è progettato per visualizzare informazioni sui file di sketch. Il costruttore configura i dettagli del comando, inclusi il nome, la descrizione e le opzioni disponibili.

#### Inizializzazione dei Membri

- **`name = "info";`**  
  Imposta il nome del comando su `info`. Questo valore viene utilizzato per identificare il comando quando viene eseguito o visualizzato.

- **`summary = "Display information about sketch files.";`**  
  Fornisce una breve descrizione del comando, che riassume la sua funzione principale: visualizzare informazioni sui file di sketch.

- **`description = "Display information about sketch files.";`**  
  Fornisce una descrizione dettagliata del comando. Specifica che il comando visualizza informazioni sui file di sketch.

- **`argumentString = "<sketch>";`**  
  Definisce la sintassi degli argomenti che il comando accetta. In questo caso, `<sketch>` rappresenta un file di sketch.

#### Opzioni del Comando

- **`useOption("help");`**  
  Aggiunge un'opzione di aiuto al comando. Quando viene specificata l'opzione `help`, viene visualizzata una guida sull'uso del comando.

- **`addOption("header", Option(Option::Boolean, "H", "", "Only show header info. Do not list each sketch. Incompatible with -d, -t and -c.", ""));`**  
  Aggiunge un'opzione denominata `header` al comando. Se l'opzione è specificata, mostra solo le informazioni dell'intestazione. Questa opzione è incompatibile con `-d`, `-t` e `-c`.

- **`addOption("tabular", Option(Option::Boolean, "t", "", "Tabular output (rather than padded), with no header. Incompatible with -d, -H and -c.", ""));`**  
  Aggiunge un'opzione denominata `tabular` al comando. Se l'opzione è specificata, fornisce un output tabellare senza intestazione. Questa opzione è incompatibile con `-d`, `-H` e `-c`.

- **`addOption("counts", Option(Option::Boolean, "c", "", "Show hash count histograms for each sketch. Incompatible with -d, -H and -t.", ""));`**  
  Aggiunge un'opzione denominata `counts` al comando. Se l'opzione è specificata, mostra gli istogrammi dei conteggi degli hash per ogni schizzo. Questa opzione è incompatibile con `-d`, `-H` e `-t`.

- **`addOption("dump", Option(Option::Boolean, "d", "", "Dump sketches in JSON format. Incompatible with -H, -t, and -c.", ""));`**  
  Aggiunge un'opzione denominata `dump` al comando. Se l'opzione è specificata, esporta gli schizzi in formato JSON. Questa opzione è incompatibile con `-H`, `-t` e `-c`.

---

### Funzione `CommandInfo::run`

#### Descrizione

La funzione `run` è il punto di ingresso principale per l'esecuzione del comando `info`. Questa funzione verifica le opzioni, gestisce i conflitti tra opzioni incompatibili, inizializza lo sketch e stampa le informazioni in base alle opzioni specificate.

#### Comportamento

1. **Verifica delle Opzioni**:
   - Controlla se le opzioni `header`, `tabular`, `counts`, e `dump` sono attive e gestisce i conflitti tra di esse.
   - Se entrambe le opzioni `header` e `tabular` sono attive, emette un errore poiché queste opzioni sono incompatibili.

2. **Inizializzazione dello Sketch**:
   - Se l'opzione `header` è attiva, inizializza solo i parametri dello sketch dal file Cap'n Proto.
   - Altrimenti, inizializza lo sketch dai file specificati.

3. **Gestione delle Opzioni Specifiche**:
   - Se l'opzione `counts` è attiva, chiama la funzione `printCounts` per stampare gli istogrammi dei conteggi degli hash.
   - Se l'opzione `dump` è attiva, chiama la funzione `writeJson` per esportare gli schizzi in formato JSON.
   - Se l'opzione `tabular` è attiva, stampa le informazioni degli schizzi in formato tabellare.
   - Se nessuna di queste opzioni è attiva, stampa le informazioni dell'intestazione e dei singoli schizzi.

#### Valore di Ritorno

- **`int`**: Restituisce `0` se l'operazione è completata con successo, `1` in caso di errore.

---

### Funzione `CommandInfo::printCounts`

#### Descrizione

La funzione `printCounts` stampa un istogramma delle frequenze degli hash per ogni schizzo. Verifica che lo sketch contenga dati e calcola l'istogramma dei conteggi degli hash.

#### Parametri

- **`sketch`** (`const Sketch &`): Lo sketch per cui stampare i conteggi.

#### Valore di Ritorno

- **`int`**: Restituisce `0` se l'operazione è completata con successo, `1` in caso di errore.

---

### Funzione `CommandInfo::writeJson`

#### Descrizione

La funzione `writeJson` esporta lo sketch in formato JSON. Serializza i dati dello sketch e li stampa in formato JSON.

#### Parametri

- **`sketch`** (`const Sketch &`): Lo sketch da esportare.

#### Valore di Ritorno

- **`int`**: Restituisce `0` se l'operazione è completata con successo, `1` in caso di errore.

---

## File `CommandInfo.h`

### Descrizione Generale

Il file `CommandInfo.h` definisce la classe `CommandInfo`, che estende la classe base `Command`. Questa classe è progettata per gestire un comando specifico all'interno di un sistema che opera su sketch e comandi associati.

### Macro di Protezione

- **`#ifndef INCLUDED_CommandInfo`**  
  **`#define INCLUDED_CommandInfo`**  
  Utilizza una macro di protezione per evitare inclusioni multiple del file.

### Dichiarazione della Classe `CommandInfo`

#### Metodi Pubblici

- **`CommandInfo()`**  
  Costruttore della classe `CommandInfo`. Inizializza il comando `info` con le opzioni disponibili.

- **`int run() const`**  
  Esegue il comando `info`. Verifica le opzioni e gestisce i conflitti tra opzioni incompatibili. Inizializza lo sketch e stampa le informazioni in base alle opzioni specificate.

#### Metodi Privati

- **`int printCounts(const Sketch & sketch) const`**  
  Stampa un istogramma delle frequenze degli hash per ogni schizzo.

- **`int writeJson(const Sketch & sketch) const`**  
  Esporta lo sketch in formato JSON.

---

## Utilizzo del Comando `info`

Esempio di utilizzo del comando `info`:

```bash
./mash info -H my_sketch.msh
```

Questo comando visualizzerà solo le informazioni dell'intestazione del file di sketch `my_sketch.msh`.

Opzioni disponibili:

- **`-H`**: Mostra solo le informazioni dell'intestazione. Incompatibile con `-d`, `-t` e `-c`.
- **`-t`**: Output tabellare, senza intestazione. Incompatibile con `-d`, `-H` e `-c`.
- **`-c`**: Mostra gli istogrammi dei conteggi degli hash per ogni schizzo. Incompatibile con `-d`, `-H` e `-t`.
- **`-d`**: Esporta gli schizzi in formato JSON. Incompatibile con `-H`, `-t` e `-c`.
