// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#ifndef INCLUDED_Command
#define INCLUDED_Command

#include <map>
#include <string>
#include <vector>
#include <set>

namespace mash {

// La classe Command è una base per la gestione dell'analisi della riga di comando
// e dei messaggi di utilizzo. I comandi specifici possono essere implementati
// estendendo questa classe e sovrascrivendo il metodo run(). Il metodo run()
// verrà chiamato da questa classe base tramite run(argc, argv) una volta che
// le opzioni sono state analizzate, e le opzioni possono essere accessibili
// dalla mappa 'options' usando le stringhe dei nomi date al momento dell'aggiunta.
// Gli argomenti (ovvero gli operandi forniti dopo le opzioni nella riga di comando)
// saranno disponibili nel vettore 'arguments'.

class Command
{
public:
    // La classe interna Option rappresenta un'opzione della riga di comando.
    // Contiene il tipo di opzione, una descrizione, valori di argomento e altro.
    class Option
    {
    public:
        enum Type
        {
            Boolean,   // Tipo booleano
            Number,    // Tipo numero (float)
            Integer,   // Tipo intero
            Size,      // Tipo dimensione (int)
            File,      // Tipo file (per i percorsi dei file)
            String     // Tipo stringa
        } type;      // Tipo dell'opzione
        
        std::string category;       // Categoria dell'opzione
        std::string identifier;    // Identificatore dell'opzione (per l'input della riga di comando)
        std::string description;   // Descrizione dell'opzione
        std::string argument;      // Argomento fornito per l'opzione
        std::string argumentDefault; // Valore predefinito dell'argomento
        
        float argumentAsNumber;    // Valore dell'argomento interpretato come numero
        float argumentMin;         // Minimo valore valido per l'argomento (per numeri)
        float argumentMax;         // Massimo valore valido per l'argomento (per numeri)
        
        bool active;               // Indica se l'opzione è attiva
        bool changed;              // Indica se l'opzione è stata modificata
        
        Option() {}
        // Costruttore della classe Option
        Option(Type typeNew, std::string identifierNew, std::string categoryNew, 
                std::string descriptionNew, std::string argumentDefaultNew = "", 
                float argumentMinNew = 0, float argumentMaxNew = 0);
        
        float getArgumentAsNumber() const {return argumentAsNumber;}
        void setArgument(std::string argumentNew);
    };

    Command();
    virtual ~Command() {};
    
    // Aggiunge un'opzione alla mappa delle opzioni.
    void addOption(std::string name, Option option);
    
    // Restituisce un riferimento costante all'opzione con il nome specificato.
    const Option & getOption(std::string name) const;
    
    // Verifica se un'opzione con il nome specificato è presente.
    inline bool hasOption(std::string name) const {return options.count(name);}
    
    // Stampa l'uso e la descrizione del comando.
    void print() const;
    
    // Metodo virtuale puro che deve essere implementato dalle classi derivate.
    virtual int run() const = 0;
    
    // Analizza e gestisce le opzioni da riga di comando.
    int run(int argc, const char ** argv);
    
    std::string name;           // Nome del comando
    std::string summary;        // Sommario del comando
    std::string description;    // Descrizione dettagliata del comando
    std::string argumentString; // Stringa che rappresenta gli argomenti del comando
    
protected:
    // Usa un'opzione predefinita
    void useOption(std::string name);
    
    // Usa le opzioni predefinite per il "sketching"
    void useSketchOptions();
    
    std::map<std::string, Option> options;        // Mappa delle opzioni attive
    std::map<std::string, Option> optionsAvailable; // Mappa delle opzioni disponibili
    std::vector<std::string> arguments;           // Vettore degli argomenti della riga di comando
    
private:
    // Aggiunge un'opzione alla lista delle opzioni disponibili
    void addAvailableOption(std::string name, Option option);
    
    // Aggiunge una categoria per organizzare le opzioni
    void addCategory(std::string name, std::string displayName);
    
    std::map<std::string, std::string> optionNamesByIdentifier; // Mappa tra identificatori e nomi delle opzioni
    std::map<std::string, std::vector<std::string> > optionNamesByCategory; // Mappa tra categorie e opzioni
    std::vector<std::string> categories; // Vettore delle categorie di opzioni
    std::map<std::string, std::string> categoryDisplayNames; // Mappa tra nomi delle categorie e nomi visualizzati
};

// Restituisce un riferimento costante all'opzione con il nome specificato.
inline const Command::Option & Command::getOption(std::string name) const {return options.at(name);}

// Legge un file e separa il contenuto in righe.
void splitFile(const std::string & file, std::vector<std::string> & lines);

// Stampa le colonne con una formattazione specificata.
void printColumns(const std::vector<std::vector<std::string>> & columns, 
                   int indent = 2, int spacing = 2, const char * missing = "-", 
                   int max = 80);

// Stampa le colonne con divider opzionali.
void printColumns(const std::vector<std::vector<std::string>> & columns, 
                   const std::vector<std::pair<int, std::string>> & dividers, 
                   int indent = 2, int spacing = 2, const char * missing = "-", 
                   int max = 80);

} // namespace mash

#endif

