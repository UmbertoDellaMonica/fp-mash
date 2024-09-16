# CommandContain

## Descrizione Generale

Il comando `CommandContain` fa parte di un programma di bioinformatica utilizzato per stimare il "containment" (cioè la frazione di sovrapposizione) tra sequenze di query e sequenze di riferimento, basato sull'uso di "sketches". Gli sketches sono rappresentazioni compatte di insiemi di k-mer (sottosequenze di lunghezza fissa) estratti dalle sequenze. Questo comando confronta le sequenze di query con quelle di riferimento, calcola la frazione di min-hashes (elementi degli sketches) condivisi tra i due insiemi, e produce un punteggio che indica quanto ogni sequenza di query è contenuta in una sequenza di riferimento.

Le principali funzionalità del comando includono la gestione dei file di input e output, l'esecuzione parallela dei confronti tra sketch, e la gestione delle opzioni del programma per personalizzare l'esecuzione.

## File `CommandContain.cpp`

Ecco la documentazione dettagliata per la sezione del file `CommandContain.cpp` che hai fornito. La documentazione è scritta in formato Markdown e si concentra esclusivamente sulla parte di codice che hai incollato:

# Documentazione del File `CommandContain.cpp`

## Descrizione Generale

Il file `CommandContain.cpp` implementa la classe `CommandContain`, che è un comando specifico per stimare il "containment" delle sequenze di query all'interno di sequenze di riferimento. Questo comando fa parte di un sistema che utilizza "sketches" per rappresentare e confrontare sequenze di DNA o RNA in modo efficiente.

## Costruttore della Classe `CommandContain`

### `CommandContain::CommandContain()`

#### Descrizione

Il costruttore della classe `CommandContain` inizializza il comando `contain`. Questo costruttore configura il comando con il nome, il sommario, la descrizione, e le opzioni disponibili. Eredita dalla classe base `Command`.

#### Dettagli di Implementazione

- **Nome del Comando**

  ```cpp
  name = "contain";
  ```

  - Il comando viene chiamato `"contain"`.

- **Sommario**

  ```cpp
  summary = "Estimate the containment of query sequences within references.";
  ```

  - Fornisce una breve descrizione del comando, indicando che il comando stima il containment delle sequenze di query all'interno delle sequenze di riferimento.

- **Descrizione**

  ```cpp
  description = "Estimate the containment of each query file (or sequence with -i) in the reference. "
                "Both the reference and queries can be fasta or fastq, gzipped or not, or mash sketch files (.msh) "
                "with matching k-mer sizes. Query files can also be files of file names (see -l). "
                "The score is the number of intersecting min-hashes divided by the query set size. "
                "The output format is [score, error-bound, reference-ID, query-ID].";
  ```

  - Fornisce una descrizione dettagliata su come il comando calcola il containment, che può coinvolgere file in formato FASTA, FASTQ, gzipped, o file di sketch `.msh`. La descrizione specifica anche il formato dell'output, che include il punteggio, il bound dell'errore, e gli ID di riferimento e query.

- **Argomenti**

  ```cpp
  argumentString = "<reference> <query> [<query>] ...";
  ```

  - Indica che il comando si aspetta almeno un file di riferimento e uno o più file di query.

- **Opzioni**

  - **Opzione "list"**

    ```cpp
    addOption("list", Option(Option::Boolean, "l", "Input", "List input. Each query file contains a list of sequence files, one per line. The reference file is not affected.", ""));
    ```

    - Tipo: Booleano
    - Descrizione: Indica se i file di query contengono una lista di file di sequenze, uno per linea. Questa opzione non influisce sul file di riferimento.

  - **Opzione "errorThreshold"**

    ```cpp
    addOption("errorThreshold", Option(Option::Number, "e", "Output", "Error bound threshold for reporting scores values. Error bounds can generally be increased by increasing the sketch size of the reference.", "0.05"));
    ```

    - Tipo: Numerico
    - Descrizione: Specifica la soglia dell'errore massimo accettabile per la segnalazione dei valori di score. Maggiore è la dimensione dello sketch del riferimento, più basso può essere il bound dell'errore. Il valore predefinito è `0.05`.

  - **Opzione "help"**

    ```cpp
    useOption("help");
    ```

    - Tipo: Predefinito
    - Descrizione: Fornisce informazioni sull'uso del comando.

  - **Opzioni Relative agli Sketch**

    ```cpp
    useSketchOptions();
    ```

    - Descrizione: Aggiunge le opzioni relative alla configurazione degli sketch, inclusi parametri come la dimensione del k-mer.

    Ecco la documentazione dettagliata per la funzione `run` del file `CommandContain.cpp` in formato Markdown:

---

## Funzione `CommandContain::run()`

### Descrizione

La funzione `run` è il cuore del comando `contain`, che esegue il calcolo del containment tra sequenze di query e riferimenti. La funzione gestisce l'intero processo, dalla verifica degli argomenti e delle opzioni, all'inizializzazione dei parametri e degli oggetti necessari, fino all'esecuzione parallela del calcolo e alla scrittura dell'output.

### Dettagli di Implementazione

#### 1. Inizio del Programma

```cpp
if ( arguments.size() < 2 || options.at("help").active )
{
    print();  // Stampa il messaggio di aiuto.
    return 0; // Esce dalla funzione con codice di successo.
}
```

- **Controllo degli Argomenti**: Verifica se ci sono almeno due argomenti passati alla linea di comando e se l'opzione "help" è attiva. Se una delle condizioni è vera, stampa il messaggio di aiuto e termina l'esecuzione con un codice di successo.

#### 2. Setup dei Parametri

```cpp
int threads = options.at("threads").getArgumentAsNumber();
bool list = options.at("list").active;

Sketch::Parameters parameters;
parameters.error = options.at("errorThreshold").getArgumentAsNumber();  // Imposta la soglia di errore.

if ( sketchParameterSetup(parameters, *(Command *)this) )
{
    return 1; // Esce dalla funzione con codice di errore.
}
```

- **Configurazione dei Parametri**: Ottiene il numero di thread e verifica se l'opzione "list" è attiva. Configura i parametri per lo sketching, inclusa la soglia di errore. Se il setup fallisce, la funzione termina con un codice di errore.

#### 3. Gestione dello Sketch di Riferimento

```cpp
Sketch sketchRef;
const string & fileReference = arguments[0];  // Il primo argomento è il file di riferimento.

bool isSketch = hasSuffix(fileReference, suffixSketch);

if ( isSketch )
{
    if ( options.at("kmer").active )
    {
        cerr << "ERROR: The option " << options.at("kmer").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
        return 1;
    }
    
    // Altri controlli per opzioni incompatibili con lo sketch...
}
else
{
    cerr << "Sketching " << fileReference << " (provide sketch file made with \"mash sketch\" to skip)...";
}
```

- **Verifica del File di Riferimento**: Determina se il file di riferimento è già uno sketch. Se è uno sketch, verifica che le opzioni non siano incompatibili. Se non è uno sketch, il file viene processato per creare uno sketch.

#### 4. Inizializzazione dello Sketch

```cpp
vector<string> refArgVector;
refArgVector.push_back(fileReference);

sketchRef.initFromFiles(refArgVector, parameters);

if ( isSketch )
{
    // Eredita i parametri dallo sketch...
}
else
{
    cerr << "done.\n";
}
```

- **Inizializzazione dello Sketch**: Aggiunge il file di riferimento a un vettore e inizializza lo sketch del riferimento. Se il file è uno sketch, eredita i parametri dallo sketch; altrimenti, completa il processo di sketching e informa l'utente.

#### 5. Pool di Thread

```cpp
ThreadPool<ContainInput, ContainOutput> threadPool(contain, threads);
```

- **Creazione del Pool di Thread**: Crea un `ThreadPool` per gestire l'esecuzione parallela del calcolo di containment. Questo permette di processare grandi dataset in modo efficiente.

#### 6. Gestione delle Query

```cpp
vector<string> queryFiles;

for ( int i = 1; i < arguments.size(); i++ )
{
    if ( list )
    {
        splitFile(arguments[i], queryFiles);
    }
    else
    {
        queryFiles.push_back(arguments[i]);
    }
}
```

- **Preparazione dei File di Query**: Popola un vettore di file di query. Se l'opzione "list" è attiva, i file di query vengono divisi in più file di sequenze.

#### 7. Inizializzazione dello Sketch della Query

```cpp
Sketch sketchQuery;
sketchQuery.initFromFiles(queryFiles, parameters, 0, true, true);
```

- **Inizializzazione dello Sketch della Query**: Inizializza lo sketch delle query utilizzando i file di query e i parametri configurati.

#### 8. Calcolo delle Coppie e Esecuzione del Thread Pool

```cpp
uint64_t pairCount = sketchRef.getReferenceCount() * sketchQuery.getReferenceCount();
uint64_t pairsPerThread = pairCount / parameters.parallelism;

if ( pairsPerThread == 0 )
{
    pairsPerThread = 1;
}

static uint64_t maxPairsPerThread = 0x1000;

if ( pairsPerThread > maxPairsPerThread )
{
    pairsPerThread = maxPairsPerThread;
}

uint64_t iFloor = pairsPerThread / sketchRef.getReferenceCount();
uint64_t iMod = pairsPerThread % sketchRef.getReferenceCount();

for ( uint64_t i = 0, j = 0; i < sketchQuery.getReferenceCount(); i += iFloor, j += iMod )
{
    if ( j >= sketchRef.getReferenceCount() )
    {
        if ( i == sketchQuery.getReferenceCount() - 1 )
        {
            break;
        }
        
        i++;
        j -= sketchRef.getReferenceCount();
    }
    
    threadPool.runWhenThreadAvailable(new ContainInput(sketchRef, sketchQuery, j, i, pairsPerThread, parameters));
    
    while ( threadPool.outputAvailable() )
    {
        writeOutput(threadPool.popOutputWhenAvailable(), parameters.error);
    }
}
```

- **Calcolo delle Coppie e Gestione dei Thread**: Calcola il numero totale di coppie di riferimenti e query, e determina quante coppie ogni thread deve processare. Limita il numero massimo di coppie per thread e assegna i lavori ai thread disponibili. Scrive l'output non appena è disponibile.

#### 9. Termine dell'Esecuzione

```cpp
while ( threadPool.running() )
{
    writeOutput(threadPool.popOutputWhenAvailable(), parameters.error);
}
```

- **Completamento dei Thread**: Assicura che tutti i thread completino il loro lavoro e scrive l'output finale prima di terminare l'esecuzione con un codice di successo.

### Conclusione

La funzione `run` gestisce l'intero flusso di lavoro del comando `contain`, inclusi la configurazione iniziale, l'inizializzazione degli oggetti necessari, l'esecuzione parallela e la scrittura dell'output. Assicura che l'operazione sia eseguita in modo efficiente e che tutti i thread completino il loro lavoro prima della conclusione.

---

Ecco la documentazione dettagliata per la funzione `writeOutput` del file `CommandContain.cpp` in formato Markdown:

---

## Funzione `CommandContain::writeOutput(ContainOutput *output, float error) const`

### Descrizione Generale

La funzione `writeOutput` si occupa di elaborare e stampare i risultati di comparazione tra sketch di query e di riferimento, forniti in un oggetto `ContainOutput`. La funzione filtra i risultati basati su una soglia di errore specificata e gestisce l'avanzamento degli indici per il confronto di tutte le coppie di risultati. Infine, si occupa di liberare la memoria utilizzata per l'oggetto `ContainOutput`.

### Dettagli di Implementazione

#### 1. Inizializzazione degli Indici

```cpp
uint64_t i = output->indexQuery;  // Indice dello sketch di query
uint64_t j = output->indexRef;    // Indice dello sketch di riferimento
```

- **Indici di Tracciamento**: Gli indici `i` e `j` sono inizializzati con gli indici degli sketch di query e di riferimento rispettivamente. Questi indici sono utilizzati per scorrere attraverso gli sketch mentre vengono processati i risultati.

#### 2. Iterazione sui Risultati

```cpp
for ( uint64_t k = 0; k < output->pairCount && i < output->sketchQuery.getReferenceCount(); k++ )
{
    const ContainOutput::PairOutput *pair = &output->pairs[k];
    
    if ( pair->error <= error )
    {
        cout << pair->score << '\t' << pair->error << '\t' << output->sketchRef.getReference(j).name << '\t' << output->sketchQuery.getReference(i).name << endl;
    }
    
    j++;
    
    if ( j == output->sketchRef.getReferenceCount() )
    {
        j = 0; // Reimposta l'indice del riferimento
        i++;   // Avanza all'indice del prossimo sketch di query
    }
}
```

- **Elaborazione dei Risultati**: La funzione itera attraverso tutte le coppie di risultati (`pairCount`) e verifica se l'errore associato a ciascuna coppia è inferiore o uguale alla soglia di errore fornita. Se il risultato è valido, vengono stampati i dettagli della comparazione:
  - **Punteggio** (`pair->score`): Il punteggio della comparazione.
  - **Errore** (`pair->error`): L'errore associato alla coppia.
  - **Nome dello Sketch di Riferimento** (`output->sketchRef.getReference(j).name`): Nome dello sketch di riferimento.
  - **Nome dello Sketch di Query** (`output->sketchQuery.getReference(i).name`): Nome dello sketch di query.

- **Avanzamento degli Indici**: L'indice del riferimento (`j`) viene incrementato ad ogni iterazione. Se `j` raggiunge il numero totale di riferimenti, viene reimpostato a zero e l'indice dello sketch di query (`i`) viene incrementato per passare al prossimo sketch di query.

#### 3. Pulizia della Memoria

```cpp
delete output;
```

- **Deallocazione della Memoria**: Alla fine della funzione, l'oggetto `output` viene eliminato per prevenire fughe di memoria e garantire che le risorse allocate siano correttamente liberate.

### Conclusione

La funzione `writeOutput` gestisce il filtraggio e la stampa dei risultati di comparazione tra sketch di query e di riferimento. Utilizza una soglia di errore per determinare quali risultati includere e gestisce l'avanzamento degli indici per coprire tutte le combinazioni di query e riferimento. Inoltre, si assicura che la memoria utilizzata venga deallocata correttamente per evitare perdite di memoria.

---

Ecco la documentazione dettagliata per la funzione `contain` del file `CommandContain.cpp` in formato Markdown:

---

## Funzione `CommandContain::contain(CommandContain::ContainInput * input)`

### Descrizione Generale

La funzione `contain` calcola la stima di containment (inclusione) tra una sequenza query e una sequenza di riferimento, utilizzando rappresentazioni compatte chiamate "sketch". Il risultato di questa funzione è un oggetto `ContainOutput` che contiene il punteggio di containment e un errore associato per ogni coppia di sequenze comparate.

### Dettagli di Implementazione

#### 1. Riferimenti agli Sketch

```cpp
const Sketch & sketchRef = input->sketchRef;
const Sketch & sketchQuery = input->sketchQuery;
```

- **Riferimenti agli Sketch**: La funzione accede agli sketch di riferimento e di query passati attraverso l'oggetto `ContainInput`. Gli sketch sono rappresentazioni compatte delle sequenze che vengono utilizzate per calcolare il containment.

#### 2. Creazione dell'Oggetto di Output

```cpp
CommandContain::ContainOutput * output = new CommandContain::ContainOutput(
    input->sketchRef,     // Sketch di riferimento
    input->sketchQuery,   // Sketch di query
    input->indexRef,      // Indice della sequenza di riferimento corrente
    input->indexQuery,    // Indice della sequenza di query corrente
    input->pairCount      // Numero di coppie (sequenze) da confrontare
);
```

- **Inizializzazione dell'Output**: Viene creato un nuovo oggetto `ContainOutput` che conterrà i risultati dei confronti di containment. L'oggetto viene inizializzato con gli sketch di riferimento e di query, gli indici delle sequenze da confrontare, e il numero di coppie da elaborare.

#### 3. Calcolo del Containment

```cpp
for ( uint64_t k = 0; k < input->pairCount && i < sketchQuery.getReferenceCount(); k++ )
{
    output->pairs[k].score = containSketches(
        sketchRef.getReference(j).hashesSorted,  // Hash della sequenza di riferimento
        sketchQuery.getReference(i).hashesSorted,// Hash della sequenza di query
        output->pairs[k].error                   // Errore del calcolo
    );
    
    j++;
    
    if ( j == sketchRef.getReferenceCount() )
    {
        j = 0;
        i++;
    }
}
```

- **Ciclo di Comparazione**: La funzione itera attraverso tutte le coppie di sequenze da confrontare. Per ogni coppia, il punteggio di containment e l'errore vengono calcolati utilizzando la funzione `containSketches`. L'errore è associato al punteggio e viene memorizzato nell'oggetto `ContainOutput`.
  - **Passaggio alla Sequenza di Riferimento Successiva**: L'indice del riferimento (`j`) viene incrementato ad ogni iterazione. Se si raggiunge il numero totale di sequenze di riferimento, l'indice viene reimpostato a zero e l'indice della sequenza di query (`i`) viene incrementato per passare alla sequenza di query successiva.

#### 4. Restituzione dell'Output

```cpp
return output;
```

- **Restituzione dell'Output**: Alla fine del ciclo, la funzione restituisce l'oggetto `ContainOutput` che contiene tutti i risultati del calcolo di containment.

### Conclusione

La funzione `contain` calcola il grado di inclusione tra le sequenze di query e di riferimento utilizzando i loro sketch. Elabora tutte le coppie di sequenze specificate, calcolando il punteggio di containment e l'errore associato per ciascuna coppia. I risultati vengono raccolti in un oggetto `ContainOutput`, che viene poi restituito per ulteriori elaborazioni o visualizzazioni.

---

Ecco la documentazione dettagliata per la funzione `containSketches` in formato Markdown:

---

## Funzione `containSketches`

### Descrizione Generale

La funzione `containSketches` calcola il livello di "containment" (inclusione) tra due insiemi di hash ordinati: uno rappresenta una sequenza di riferimento e l'altro una sequenza di query. Il livello di containment è espresso come un valore di tipo `double`, che rappresenta la proporzione di hash comuni tra i due insiemi. Inoltre, la funzione calcola e restituisce un valore di errore associato al calcolo.

### Dettagli di Implementazione

#### Parametri di Input

- **`const HashList & hashesSortedRef`**: Un insieme ordinato di hash che rappresenta la sequenza di riferimento.
- **`const HashList & hashesSortedQuery`**: Un insieme ordinato di hash che rappresenta la sequenza di query.
- **`double & errorToSet`**: Una variabile di riferimento in cui viene impostato il valore di errore associato al calcolo del containment.

#### Processo di Calcolo

1. **Inizializzazione**

   ```cpp
   int common = 0;  // Conta il numero di hash comuni tra riferimento e query.
   int denom = hashesSortedRef.size() < hashesSortedQuery.size() ?
       hashesSortedRef.size() :
       hashesSortedQuery.size();  // Dimensione del denominatore per il calcolo del containment.
   int i = 0;  // Indice per scorrere gli hash del riferimento.
   int j = 0;  // Indice per scorrere gli hash della query.
   ```

   - **`common`**: Variabile per contare gli hash comuni tra riferimento e query.
   - **`denom`**: Determina il numero totale di hash da confrontare, basato sulla dimensione minore tra i due insiemi.
   - **`i` e `j`**: Indici per iterare attraverso gli hash ordinati del riferimento e della query, rispettivamente.

2. **Confronto degli Hash**

   ```cpp
   for ( int steps = 0; steps < denom && i < hashesSortedRef.size(); steps++ )
   {
       if ( hashLessThan(hashesSortedRef.at(i), hashesSortedQuery.at(j), hashesSortedRef.get64()) )
       {
           i++;
           steps--;
       }
       else if ( hashLessThan(hashesSortedQuery.at(j), hashesSortedRef.at(i), hashesSortedRef.get64()) )
       {
           j++;
       }
       else
       {
           i++;
           j++;
           common++;
       }
   }
   ```

   - **Ciclo di Confronto**: Il ciclo itera attraverso gli hash ordinati e confronta gli hash della sequenza di riferimento con quelli della sequenza di query.
     - **`hashLessThan`**: Una funzione ausiliaria utilizzata per confrontare gli hash. Determina quale hash è minore tra quelli della query e del riferimento.
     - **Incremento degli Indici**: Gli indici degli hash vengono incrementati in base ai risultati del confronto.

3. **Calcolo dell'Errore**

   ```cpp
   errorToSet = 1. / sqrt(j);
   ```

   - **Errore Associato**: L'errore è calcolato come l'inverso della radice quadrata del numero di hash della query considerati. Questo valore viene memorizzato nella variabile `errorToSet`.

4. **Restituzione del Punteggio di Containment**

   ```cpp
   return double(common) / j;
   ```

   - **Punteggio di Containment**: Il punteggio di containment è il rapporto tra il numero di hash comuni (`common`) e il numero di hash della query considerati (`j`). Viene restituito come valore `double`.

### Conclusione

La funzione `containSketches` confronta due insiemi di hash ordinati, uno rappresentante una sequenza di riferimento e l'altro una sequenza di query. Calcola la proporzione di hash comuni tra i due insiemi e fornisce un valore di errore associato al calcolo. Questo punteggio di containment è utile per valutare quanto una sequenza di query è contenuta all'interno di una sequenza di riferimento.

---

Ecco la documentazione per il file `CommandContain.h` in formato Markdown:

---

## File `CommandContain.h`

### Descrizione Generale

Questo file definisce la classe `CommandContain` e le strutture associate utilizzate per calcolare e gestire i risultati del containment (inclusione) tra due insiemi di sketch. Le strutture e le funzioni dichiarate in questo file sono progettate per confrontare sequenze di dati rappresentati come hash e gestire i risultati di tali confronti.

### Classi e Strutture

#### Classe `CommandContain`

La classe `CommandContain` estende la classe base `Command` e si occupa di eseguire il calcolo del containment tra sequenze di sketch. Include metodi per gestire i risultati e processare i dati.

##### Membri Pubblici

- **`CommandContain();`**
  
  Costruttore della classe `CommandContain`. Inizializza una nuova istanza della classe.

- **`int run() const;`**

  Metodo `run` che esegue il calcolo del containment e restituisce un intero come risultato dell'operazione. Questo metodo è un'override del metodo `run` della classe base `Command`.

##### Membri Privati

- **`void writeOutput(ContainOutput * output, float error) const;`**

  Metodo privato che scrive i risultati del calcolo del containment. Prende un puntatore a un oggetto `ContainOutput` e un valore di errore come argomenti e stampa i risultati.

#### Struttura `ContainInput`

La struttura `ContainInput` incapsula i dati necessari per calcolare il containment tra due set di sketch: uno di riferimento e uno di query.

- **`ContainInput(const Sketch & sketchRefNew, const Sketch & sketchQueryNew, uint64_t indexRefNew, uint64_t indexQueryNew, uint64_t pairCountNew, const Sketch::Parameters & parametersNew);`**

  Costruttore della struttura `ContainInput`. Inizializza i membri con i valori forniti.

- **`const Sketch & sketchRef;`**

  Riferimento costante allo sketch di riferimento.

- **`const Sketch & sketchQuery;`**

  Riferimento costante allo sketch di query.

- **`uint64_t indexRef;`**

  Indice della posizione corrente nello sketch di riferimento.

- **`uint64_t indexQuery;`**

  Indice della posizione corrente nello sketch di query.

- **`uint64_t pairCount;`**

  Numero di coppie di sketch da confrontare.

- **`std::string nameRef;`**

  Nome del riferimento (non utilizzato direttamente nel costruttore o altrove).

- **`const Sketch::Parameters & parameters;`**

  Riferimento ai parametri di sketching utilizzati.

#### Struttura `ContainOutput`

La struttura `ContainOutput` raccoglie i risultati del calcolo del containment tra due sketch.

- **`ContainOutput(const Sketch & sketchRefNew, const Sketch & sketchQueryNew, uint64_t indexRefNew, uint64_t indexQueryNew, uint64_t pairCountNew);`**

  Costruttore della struttura `ContainOutput`. Inizializza i membri e alloca dinamicamente un array di `PairOutput`.

- **`~ContainOutput();`**

  Distruttore della struttura `ContainOutput`. Libera la memoria allocata per l'array di `PairOutput`.

- **`struct PairOutput`**

  Struttura interna per memorizzare i risultati di un confronto tra due sketch:
  - **`double score;`** - Punteggio del confronto tra due sketch.
  - **`double error;`** - Errore associato al calcolo del punteggio.

- **`const Sketch & sketchRef;`**

  Riferimento costante allo sketch di riferimento.

- **`const Sketch & sketchQuery;`**

  Riferimento costante allo sketch di query.

- **`uint64_t indexRef;`**

  Indice della posizione corrente nello sketch di riferimento.

- **`uint64_t indexQuery;`**

  Indice della posizione corrente nello sketch di query.

- **`uint64_t pairCount;`**

  Numero di coppie di sketch confrontate.

- **`PairOutput * pairs;`**

  Puntatore a un array di `PairOutput`, contenente i risultati di ciascun confronto.

### Funzioni

- **`CommandContain::ContainOutput * contain(CommandContain::ContainInput * data);`**

  Funzione che calcola il containment tra due insiemi di sketch e restituisce un puntatore a un oggetto `ContainOutput` contenente i risultati del confronto.

- **`double containSketches(const HashList & hashesSortedRef, const HashList & hashesSortedQuery, double & errorToSet);`**

  Funzione che calcola il livello di containment tra due insiemi di hash ordinati. Restituisce un punteggio di containment e imposta un valore di errore associato al calcolo.

---
