# CommandBounds

## Descrizione

Il comando `mash bounds` viene utilizzato per calcolare e stampare una tabella di limiti di errore per le stime delle distanze di Mash in base a diverse dimensioni di sketch e distanze di Mash. Questi calcoli sono utili per capire quanto siano accurate le stime di distanza basate su `k-mer` quando si usano i "sketches", che sono una rappresentazione compatta delle sequenze.

### Esempio di Utilizzo

Supponiamo di voler usare il comando con i seguenti parametri:

- `k-mer size` di 21 (il valore predefinito).
- Una probabilità del 99% che le stime della distanza di Mash rientrino nei limiti di errore calcolati (il valore predefinito).

Il comando sarà:

```bash
mash bounds
```

Questo comando, senza alcuna opzione, utilizza i valori predefiniti per il `k-mer size` e la probabilità.

### Personalizzare il Comando

Se vuoi cambiare i parametri, puoi usare le opzioni:

1. **Cambiare la dimensione del k-mer (`-k`)**:
   - Se vuoi calcolare i limiti di errore per un `k-mer size` diverso, ad esempio 15, usa:

     ```bash
     mash bounds -k 15
     ```

2. **Cambiare la probabilità (`-p`)**:
   - Se desideri che la probabilità sia, ad esempio, 95% anziché 99%, usa:

     ```bash
     mash bounds -p 0.95
     ```

   - In questo caso, il comando calcolerà i limiti di errore sapendo che le stime di distanza avranno un margine di errore più ampio rispetto al caso con probabilità 99%.

3. **Mostrare l'Help (`-h`)**:
   - Se vuoi vedere un riepilogo delle opzioni disponibili, puoi usare:

     ```bash
     mash bounds -h
     ```

   - Questo ti mostrerà la descrizione del comando e tutte le opzioni che puoi usare.

### Capire i Risultati

Quando esegui il comando, vedrai una tabella che include:

- **Sketch Sizes**: Le diverse dimensioni dello sketch che sono state considerate.
- **Mash Distances**: Le distanze di Mash per cui vengono calcolati i limiti di errore.
- **Error Bounds**: Per ogni combinazione di `sketch size` e `Mash distance`, verrà mostrato quanto la stima di distanza potrebbe variare rispetto alla vera distanza, in funzione della probabilità specificata.

### Applicazione Pratica

Questo comando è particolarmente utile quando stai lavorando con diverse dimensioni di sketch o stai cercando di ottimizzare il bilanciamento tra accuratezza della stima e velocità/compressione dei dati. Utilizzando questo comando, puoi determinare la dimensione minima del tuo sketch per ottenere una certa accuratezza nelle tue stime di distanza.

### Suggerimenti

- **Dimensione del k-mer (`-k`)**: Se lavori con genomi molto lunghi, puoi usare un `k-mer size` più grande per avere stime più precise.
- **Probabilità (`-p`)**: Ridurre la probabilità (ad esempio a 0.95) può essere utile se vuoi considerare una stima più conservativa della distanza.

Puoi provare diverse combinazioni di parametri per capire meglio come influenzano i limiti di errore nelle tue analisi.
