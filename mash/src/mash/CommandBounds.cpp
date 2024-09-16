// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "CommandBounds.h"
#include <iostream>
#include <math.h>

#ifdef USE_BOOST
    #include <boost/math/distributions/binomial.hpp>
    using namespace::boost::math;
#else
    #include <gsl/gsl_cdf.h>
#endif

using std::cout;
using std::endl;

namespace mash {


/**
 * Il comando `mash bounds` viene utilizzato per calcolare e stampare una tabella di limiti di errore per le stime delle distanze di Mash in base a diverse dimensioni di sketch e distanze di Mash. Questi calcoli sono utili per capire quanto siano accurate le stime di distanza basate su `k-mer` quando si usano i "sketches", che sono una rappresentazione compatta delle sequenze.

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
 */


CommandBounds::CommandBounds()
: Command()
{
    name = "bounds";
    summary = "Print a table of Mash error bounds.";
    description = "Print a table of Mash error bounds for various sketch sizes and Mash distances based on a given k-mer size and desired confidence. Note that these calculations assume sequences are much larger than the sketch size, and will overestimate error bounds if this is not the case.";
    argumentString = "";
    
    useOption("help");
    addOption("kmer", Option(Option::Integer, "k", "", "k-mer size.", "21", 1, 32));
    addOption("prob", Option(Option::Number, "p", "", "Mash distance estimates will be within the given error bounds with this probability.", "0.99", 0, 1));
}

int CommandBounds::run() const
{
    if ( options.at("help").active )
    {
        print();
        return 0;
    }
    
	const int sketchSizeCount = 9;
	const double sketchSizes[] = {100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
	
	const int distCount = 8;
	const double dists[] = {0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4};
	
	int k = getOption("kmer").getArgumentAsNumber();
	double q2 = (1.0 - getOption("prob").getArgumentAsNumber()) / 2.0;
	
	cout << endl << "Parameters (run with -h for details):" << endl;
	cout << "   k:   " << k << endl;
	cout << "   p:   " << getOption("prob").getArgumentAsNumber() << endl << endl;
	
	for ( int cont = 0; cont < 2; cont++ )
	{
		if ( cont )
		{
			cout << "\tScreen distance" << endl;
		}
		else
		{
			cout << "\tMash distance" << endl;
		}
		
		cout << "Sketch";
	
		for ( int i = 0; i < distCount; i++ )
		{
			cout << '\t' << dists[i];
		}
	
		cout << endl;
	
		for ( int i = 0; i < sketchSizeCount; i++ )
		{
			int s = sketchSizes[i];
			cout << s;
		
			for ( int j = 0; j < distCount; j++ )
			{
				double m2j;
				
				if ( cont )
				{
					//m2j = exp(-k * dists[j]);
					m2j = pow(1.0 - dists[j], k); // binomial model
				}
				else
				{
					m2j = 1.0 / (2.0 * exp(k * dists[j]) - 1.0);
				}
			
				int x = 0;
			
				while ( x < s )
				{
#ifdef USE_BOOST
					double cdfx = cdf(binomial(s, m2j), x);
#else
					double cdfx = gsl_cdf_binomial_P(x, m2j, s);
#endif
					if ( cdfx > q2 )
					{
						break;
					}
				
					x++;
				}
			
				double je = double(x) / s;
				double j2m;
				
				if ( cont )
				{
					//j2m = -1.0 / k * log(je);
					j2m = 1.0 - pow(je, 1. / k);
				}
				else
				{
					j2m = -1.0 / k * log(2.0 * je / (1.0 + je));
				}
				
				cout << '\t' << j2m - dists[j];
			}
		
			cout << endl;
		}
	
		cout << endl;
	}
	
	return 0;
}

} // namespace mash


/**Questo codice C++ fa parte del software **Mash**, che è utilizzato per stimare distanze genomiche basate su k-mer (sotto-sequenze di lunghezza k) utilizzando una tecnica chiamata **sketching**. In particolare, questa sezione di codice implementa il comando `bounds` di Mash, il quale calcola e stampa una tabella di limiti di errore per le stime di distanza di Mash, basandosi su diverse dimensioni di sketch e distanze di Mash.

Ecco una spiegazione più dettagliata delle varie parti del codice:

### 1. **Inclusione delle Librerie**
   ```cpp
   #ifdef USE_BOOST
       #include <boost/math/distributions/binomial.hpp>
       using namespace::boost::math;
   #else
       #include <gsl/gsl_cdf.h>
   #endif
   ```
   Qui viene scelto se utilizzare la libreria Boost o la libreria GSL per il calcolo delle distribuzioni binomiali. Se `USE_BOOST` è definito, il codice utilizza Boost, altrimenti GSL.

### 2. **Costruttore `CommandBounds::CommandBounds`**
   ```cpp
   CommandBounds::CommandBounds()
   : Command()
   {
       name = "bounds";
       summary = "Print a table of Mash error bounds.";
       description = "Print a table of Mash error bounds for various sketch sizes and Mash distances based on a given k-mer size and desired confidence. Note that these calculations assume sequences are much larger than the sketch size, and will overestimate error bounds if this is not the case.";
       argumentString = "";
       
       useOption("help");
       addOption("kmer", Option(Option::Integer, "k", "", "k-mer size.", "21", 1, 32));
       addOption("prob", Option(Option::Number, "p", "", "Mash distance estimates will be within the given error bounds with this probability.", "0.99", 0, 1));
   }
   ```
   Questo è il costruttore della classe `CommandBounds`. Configura il comando `bounds` assegnandogli un nome (`name`), un sommario (`summary`), e una descrizione (`description`). Vengono anche aggiunte delle opzioni:
   - `kmer`: la dimensione del k-mer (default: 21).
   - `prob`: la probabilità che le stime di distanza di Mash rientrino nei limiti di errore (default: 0.99).

### 3. **Funzione `run()`**
   ```cpp
   int CommandBounds::run() const
   ```
   Questa funzione esegue il comando quando viene chiamato. Il flusso principale del codice è suddiviso in varie sezioni.

   #### a. **Gestione dell'opzione `help`**
   ```cpp
   if (options.at("help").active)
   {
       print();
       return 0;
   }
   ```
   Se l'opzione `help` è attiva, viene stampata l'help del comando e il programma termina.

   #### b. **Impostazioni dei Parametri**
   ```cpp
   const int sketchSizeCount = 9;
   const double sketchSizes[] = {100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
   
   const int distCount = 8;
   const double dists[] = {0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4};
   
   int k = getOption("kmer").getArgumentAsNumber();
   double q2 = (1.0 - getOption("prob").getArgumentAsNumber()) / 2.0;
   ```
   Qui sono definiti:
   - `sketchSizes`: un array contenente diverse dimensioni di sketch.
   - `dists`: un array contenente diverse distanze di Mash.
   - `k`: la dimensione del k-mer.
   - `q2`: metà della probabilità complementare a `prob`, utilizzata nei calcoli successivi.

   #### c. **Stampa dei Parametri**
   ```cpp
   cout << endl << "Parameters (run with -h for details):" << endl;
   cout << "   k:   " << k << endl;
   cout << "   p:   " << getOption("prob").getArgumentAsNumber() << endl << endl;
   ```
   Vengono stampati i parametri selezionati (`k` e `prob`).

   #### d. **Calcolo e Stampa dei Limiti di Errore**
   Il codice è diviso in due parti: una per le **Mash distance** e una per le **screen distance**.

   ```cpp
   for ( int cont = 0; cont < 2; cont++ )
   ```
   - Se `cont` è 0, si calcolano le **Mash distance**.
   - Se `cont` è 1, si calcolano le **screen distance**.

   ```cpp
   for ( int i = 0; i < sketchSizeCount; i++ )
   {
       int s = sketchSizes[i];
       cout << s;
   
       for ( int j = 0; j < distCount; j++ )
       {
           double m2j;
           
           if ( cont )
           {
               m2j = pow(1.0 - dists[j], k); // screen distance
           }
           else
           {
               m2j = 1.0 / (2.0 * exp(k * dists[j]) - 1.0); // Mash distance
           }
           
           int x = 0;
           
           while ( x < s )
           {
   #ifdef USE_BOOST
               double cdfx = cdf(binomial(s, m2j), x);
   #else
               double cdfx = gsl_cdf_binomial_P(x, m2j, s);
   #endif
               if ( cdfx > q2 )
               {
                   break;
               }
               
               x++;
           }
           
           double je = double(x) / s;
           double j2m;
           
           if ( cont )
           {
               j2m = 1.0 - pow(je, 1. / k); // screen distance
           }
           else
           {
               j2m = -1.0 / k * log(2.0 * je / (1.0 + je)); // Mash distance
           }
           
           cout << '\t' << j2m - dists[j];
       }
       
       cout << endl;
   }
   ```
   - Per ogni dimensione di sketch (`s`), vengono calcolati e stampati i limiti di errore per ciascuna distanza (`dists[j]`).
   - **Mash distance**: Si utilizza una funzione esponenziale per modellare la distanza.
   - **Screen distance**: Si utilizza un modello binomiale.

### 4. **Funzione di chiusura**
   ```cpp
   return 0;
   ```
   Il comando `bounds` termina restituendo 0, indicando un'esecuzione corretta.

### In sintesi

Questo codice calcola e stampa una tabella che mostra i limiti di errore per diverse dimensioni di sketch e stime di distanza di Mash, basandosi sui parametri forniti (dimensione del k-mer e probabilità). Le funzioni matematiche, come quelle legate alla distribuzione binomiale, vengono utilizzate per determinare la precisione delle stime di distanza di Mash. */