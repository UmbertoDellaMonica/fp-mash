Ecco una spiegazione dettagliata e ben formattata delle due funzioni di hash MurmurHash3, suddivisa con un separatore:

---

## MurmurHash3_x86_32

`MurmurHash3_x86_32` è una funzione di hash progettata per generare un hash a 32 bit su piattaforme a 32 bit. Il codice funziona come segue:

### Struttura del Codice

1. **Definizioni e Dichiarazioni**:
   - **Input**:
     - `key`: Puntatore ai dati di input.
     - `len`: Lunghezza dei dati di input in byte.
     - `seed`: Valore iniziale usato per diversificare l'hash.
     - `out`: Puntatore alla memoria dove l'hash calcolato sarà scritto.
   - **Variabili e Costanti**:
     - `data`: Puntatore ai dati di input come array di byte.
     - `nblocks`: Numero di blocchi di 4 byte (32 bit) nei dati di input.
     - `h1`: Variabile che contiene il valore dell'hash, inizializzata con il seed.
     - `c1` e `c2`: Costanti usate nel calcolo dell'hash.

2. **Corpo della Funzione (Body)**:
   - **Blocco di Dati**:
     - `blocks`: Puntatore ai blocchi di 4 byte (32 bit) dei dati di input.
   - **Elaborazione dei Blocchi**:
     - Per ogni blocco:
       - **k1**: Valore del blocco corrente.
       - **k1** viene moltiplicato per `c1`, ruotato a sinistra di 15 bit e moltiplicato di nuovo per `c2`.
       - **h1** viene aggiornato con il valore di `k1` usando l'operazione XOR e poi ruotato a sinistra di 13 bit.
       - **h1** viene moltiplicato per 5 e incrementato di 0xe6546b64.

3. **Gestione della Coda (Tail)**:
   - Gestisce i byte rimanenti che non formano un blocco completo.
   - **k1** viene inizializzato a 0 e poi aggiornato con i byte rimanenti.
   - **k1** viene moltiplicato per `c1`, ruotato a sinistra di 15 bit, moltiplicato per `c2` e combinato con `h1` usando l'operazione XOR.

4. **Finalizzazione (Finalization)**:
   - **h1** viene aggiornato con la lunghezza totale dei dati usando l'operazione XOR.
   - **h1** viene mescolato ulteriormente utilizzando la funzione `fmix32`.
   - L'hash finale viene scritto nella memoria puntata da `out`.

### Dettagli

1. **Inizializzazione**:

   ```cpp
   const uint8_t * data = (const uint8_t*)key;
   const int nblocks = len / 4;

   uint32_t h1 = seed;
   const uint32_t c1 = 0xcc9e2d51;
   const uint32_t c2 = 0x1b873593;
   ```

   - `data` punta ai dati di input.
   - `nblocks` calcola quanti blocchi di 4 byte ci sono.
   - `h1` è inizializzato con il seed.
   - `c1` e `c2` sono costanti utilizzate per mescolare i dati.

2. **Elaborazione dei Blocchi**:

   ```cpp
   const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

   for(int i = -nblocks; i; i++)
   {
     uint32_t k1 = getblock32(blocks,i);

     k1 *= c1;
     k1 = ROTL32(k1,15);
     k1 *= c2;

     h1 ^= k1;
     h1 = ROTL32(h1,13);
     h1 = h1*5+0xe6546b64;
   }
   ```

   - `blocks` punta ai blocchi di dati.
   - Per ogni blocco:
     - `k1` è il valore del blocco corrente.
     - `k1` viene moltiplicato per `c1`, ruotato a sinistra di 15 bit, e poi moltiplicato per `c2`.
     - `h1` viene aggiornato con il valore di `k1` usando XOR.
     - `h1` viene ruotato a sinistra di 13 bit e poi aggiornato con un'operazione di moltiplicazione e aggiunta.

3. **Gestione della Coda**:

   ```cpp
   const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

   uint32_t k1 = 0;

   switch(len & 3)
   {
   case 3: k1 ^= tail[2] << 16;
   case 2: k1 ^= tail[1] << 8;
   case 1: k1 ^= tail[0];
           k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
   };
   ```

   - `tail` punta ai byte rimanenti.
   - `k1` è aggiornato con i byte rimanenti.
   - `k1` viene mescolato con `c1` e `c2` e combinato con `h1`.

4. **Finalizzazione**:

   ```cpp
   h1 ^= len; // XOR
   h1 = fmix32(h1);
   *(uint32_t*)out = h1;
   ```

   - `h1` viene aggiornato con la lunghezza dei dati usando XOR.
   - `h1` viene mescolato ulteriormente usando `fmix32`.
   - L'hash finale viene scritto nella memoria puntata da `out`.

### Riassunto

`MurmurHash3_x86_32` è progettato per calcolare un hash a 32 bit in modo veloce e efficiente su piattaforme a 32 bit. Utilizza una serie di operazioni matematiche e bitwise per mescolare i bit dei dati di input e produrre un hash che distribuisce uniformemente i valori di hash, riducendo la probabilità di collisioni.

---

## MurmurHash3_x64_128

`MurmurHash3_x64_128` è una versione di MurmurHash3 progettata per generare un hash a 128 bit (16 byte) su piattaforme a 64 bit. Ecco come funziona il codice:

### Struttura del Codice

1. **Definizioni delle Macros e Funzioni di Supporto**:
   - **Macros**: Definiscono funzioni inline e rotazioni per diverse piattaforme.
   - **ROTL32/ROTL64**: Ruotano i bit di un numero a 32 o 64 bit di un certo numero di posizioni.
   - **BIG_CONSTANT**: Gestisce le costanti grandi.
   - **getblock32/getblock64**: Leggono blocchi di dati da un array.

2. **Funzioni di Mixing**:
   - **fmix32/fmix64**: Applicano una serie di operazioni bitwise per mescolare i bit dell'hash. Queste operazioni sono progettate per garantire che anche piccole modifiche nell'input causino grandi cambiamenti nell'hash.

### Funzione `MurmurHash3_x64_128`

#### Input

- **key**: Puntatore ai dati di input.
- **len**: Lunghezza dei dati di input in byte.
- **seed**: Un valore iniziale usato per diversificare l'hash.
- **out**: Puntatore alla memoria dove l'hash calcolato sarà scritto.

#### Corpo della Funzione

1. **Inizializzazione**:
   - **data**: Puntatore ai dati di input.
   - **nblocks**: Numero di blocchi da 128 bit nei dati di input.
   - **h1 e h2**: Inizializzati con il seed.
   - **c1 e c2**: Costanti usate nel calcolo dell'hash.

2. **Corpo (Body)**:
   - **blocks**: Puntatore ai blocchi di dati.
   - Per ogni blocco:
     - **k1 e k2**: Valori intermedi calcolati dai blocchi di dati.
     - Operazioni di mixaggio su `k1` e `k2` e aggiornamento di `h1` e `h2` con i risultati.
     - Uso delle operazioni di rotazione e moltiplicazione per mescolare i bit.

3. **Gestione della Coda (Tail)**:
   - Gestisce i byte rimanenti che non formano un blocco completo.
   - **k1 e k2**: Inizializzati a 0 e poi aggiornati con i byte rimanenti.
   - Mixaggio finale di `k1` e `k2`.

4. **Finalizzazione (Finalization)**:
   - **h1 e h2**: Mescolati ulteriormente e combinati.
   - Scrittura del risultato nei primi due elementi dell'array puntato da `out`.

### Flusso

 Dettagliato

1. **Corpo della Funzione**:
   - Dividi i dati di input in blocchi da 16 byte (128 bit).
   - Ogni blocco è diviso in due parti da 8 byte (64 bit), `k1` e `k2`.
   - `k1` e `k2` vengono mescolati utilizzando le costanti `c1` e `c2`, e operazioni di rotazione e moltiplicazione.
   - I risultati vengono combinati con `h1` e `h2`, che vengono anche mescolati.

2. **Gestione della Coda**:
   - I byte rimanenti (meno di 16) vengono mescolati in `k1` e `k2`.
   - `k1` e `k2` vengono quindi mescolati ulteriormente e combinati con `h1` e `h2`.

3. **Finalizzazione**:
   - `h1` e `h2` vengono mescolati per garantire che ogni bit dell'input abbia un'influenza sul risultato finale.
   - I risultati vengono scritti nella memoria indicata da `out`.

### Riassunto

`MurmurHash3_x64_128` è progettato per generare un hash a 128 bit in modo efficiente su piattaforme a 64 bit. Il codice utilizza operazioni di rotazione, moltiplicazione e mescolamento per garantire una distribuzione uniforme dei bit nell'output, rendendo l'hash risultante resistente alle collisioni e adatto per l'uso in tabelle hash e altre applicazioni che richiedono funzioni di hash ad alta qualità.


Certo! Ecco uno schema grafico che illustra il funzionamento di `MurmurHash3_x86_32` e `MurmurHash3_x64_128`, visualizzabile in formato Markdown.

### Schema Grafico di MurmurHash3

```markdown
## MurmurHash3_x86_32

+---------------------+
|      Input Data     |
+---------------------+
           |
           v
+---------------------+
|  Divide Data into   |
|   4-byte Blocks     |
+---------------------+
           |
           v
+---------------------+
|    Initialize       |
|    Variables:       |
|   h1 = seed         |
|   c1, c2 = constants|
+---------------------+
           |
           v
+---------------------+
|   Process Blocks    |
|   for each block:   |
|   k1 = block value  |
|   k1 = k1 * c1      |
|   k1 = ROTL32(k1,15)|
|   k1 = k1 * c2      |
|   h1 ^= k1  //XOR        |
|   h1 = ROTL32(h1,13)|
|   h1 = h1 * 5 + 0xe6546b64 |
+---------------------+
           |
           v
+---------------------+
|  Handle Remaining   |
|     Bytes (Tail)    |
|   k1 = 0            |
|   k1 ^= tail bytes  |
|   k1 = k1 * c1      |
|   k1 = ROTL32(k1,15)|
|   k1 = k1 * c2      |
|   h1 ^= k1          |
+---------------------+
           |
           v
+---------------------+
|    Finalize         |
|   h1 ^= len         |
|   h1 = fmix32(h1)   |
+---------------------+
           |
           v
+---------------------+
|    Output Hash      |
+---------------------+
```

```

## MurmurHash3_x64_128

+---------------------+
|      Input Data     |
+---------------------+
           |
           v
+---------------------+
|  Divide Data into   |
|  16-byte Blocks     |
+---------------------+
           |
           v
+---------------------+
|    Initialize       |
|    Variables:       |
|   h1, h2 = seed     |
|   c1, c2 = constants|
+---------------------+
           |
           v
+---------------------+
|   Process Blocks    |
|   for each block:   |
|   k1, k2 = block    |
|   k1 = k1 * c1      |
|   k1 = ROTL64(k1,31)|
|   k1 = k1 * c2      |
|   h1 ^= k1          |
|   h1 = ROTL64(h1,27)|
|   h1 = h1 * 5 + 0x52dce729 |
|   k2 = k2 * c2      |
|   k2 = ROTL64(k2,33)|
|   k2 = k2 * c1      |
|   h2 ^= k2          |
|   h2 = ROTL64(h2,31)|
|   h2 = h2 * 5 + 0x38495ab5 |
+---------------------+
           |
           v
+---------------------+
|  Handle Remaining   |
|     Bytes (Tail)    |
|   k1, k2 = 0        |
|   k1 ^= tail bytes  |
|   k1 = k1 * c1      |
|   k1 = ROTL64(k1,31)|
|   k1 = k1 * c2      |
|   h1 ^= k1          |
|   k2 ^= tail bytes  |
|   k2 = k2 * c2      |
|   k2 = ROTL64(k2,33)|
|   k2 = k2 * c1      |
|   h2 ^= k2          |
+---------------------+
           |
           v
+---------------------+
|    Finalize         |
|   h1 ^= len         |
|   h2 ^= len         |
|   h1 = fmix64(h1)   |
|   h2 = fmix64(h2)   |
|   Combine h1, h2    |
+---------------------+
           |
           v
+---------------------+
|    Output Hash      |
+---------------------+
```


### Descrizione dello Schema

1. **Input Data**: I dati di input sono letti e preparati per l'elaborazione.
2. **Divide Data into Blocks**: I dati vengono suddivisi in blocchi della dimensione appropriata (4-byte per `x86_32`, 16-byte per `x64_128`).
3. **Initialize Variables**: Le variabili e le costanti sono inizializzate.
4. **Process Blocks**: Ogni blocco viene elaborato utilizzando operazioni matematiche e bitwise.
5. **Handle Remaining Bytes (Tail)**: I byte rimanenti che non formano un blocco completo vengono gestiti separatamente.
6. **Finalize**: L'hash finale viene calcolato e mescolato.
7. **Output Hash**: L'hash risultante viene scritto nella memoria di output.
