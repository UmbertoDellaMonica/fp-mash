// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "CommandInfo.h"
#include "Sketch.h"
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace mash {

#ifdef ARCH_32
    #define HASH "MurmurHash3_x86_32"
#else
    #define HASH "MurmurHash3_x64_128"
#endif

/**
 * Costruttore della classe CommandInfo.
 * 
 * Inizializza il comando "info" con le opzioni disponibili:
 * - help: Mostra l'aiuto del comando.
 * - header (-H): Mostra solo le informazioni dell'intestazione. Incompatibile con -d, -t e -c.
 * - tabular (-t): Output tabellare, senza intestazione. Incompatibile con -d, -H e -c.
 * - counts (-c): Mostra gli istogrammi dei conteggi degli hash per ogni schizzo. Incompatibile con -d, -H e -t.
 * - dump (-d): Esporta gli schizzi in formato JSON. Incompatibile con -H, -t e -c.
 */
CommandInfo::CommandInfo()
: Command()
{
    name = "info";
    summary = "Display information about sketch files.";
    description = "Display information about sketch files.";
    argumentString = "<sketch>";
    
    useOption("help");
    addOption("header", Option(Option::Boolean, "H", "", 
        "Only show header info. Do not list each sketch. Incompatible with -d, -t and -c.", ""));
    addOption("tabular", Option(Option::Boolean, "t", "", 
        "Tabular output (rather than padded), with no header. Incompatible with -d, -H and -c.", ""));
    addOption("counts", Option(Option::Boolean, "c", "", 
        "Show hash count histograms for each sketch. Incompatible with -d, -H and -t.", ""));
    addOption("dump", Option(Option::Boolean, "d", "", 
        "Dump sketches in JSON format. Incompatible with -H, -t, and -c.", ""));
    
    addOption("fingerprint-dump",Option(Option::Boolean,"fpd","",
    "Dump fingerprint sketches in JSON format. Incompatibile with -H , -t , -c.", ""));
}

/**
 * Esegue il comando "info".
 * 
 * Verifica le opzioni e gestisce i conflitti tra opzioni incompatibili.
 * Inizializza lo sketch e stampa le informazioni in base alle opzioni specificate.
 * 
 * @return 0 se l'operazione è completata con successo, 1 in caso di errore.
 */
int CommandInfo::run() const
{
    if (arguments.size() != 1 || options.at("help").active)
    {
        print();
        return 0;
    }
    
    bool header = options.at("header").active;
    bool tabular = options.at("tabular").active;
    bool counts = options.at("counts").active;
    bool dump = options.at("dump").active;
    bool fingerprint = options.at("fingerprint-dump").active;

    cout<< " ACTIVE OPTION HEADER ? : "<< header <<endl;
    cout<< " ACTIVE OPTION TABULAR ? : "<< tabular <<endl;
    cout<< " ACTIVE OPTION COUNTS ? : "<< counts <<endl;
    cout<< " ACTIVE OPTION DUMP ? : "<< dump <<endl;
    cout<< " ACTIVE OPTION FINGERPRINT ? : "<< fingerprint <<endl;
    
    // Verifica delle opzioni incompatibili
    if (header && tabular)
    {
        cerr << "ERROR: The options -H and -t are incompatible." << endl;
        return 1;
    }
    
    if (header && counts)
    {
        cerr << "ERROR: The options -H and -c are incompatible." << endl;
        return 1;
    }
    
    if (tabular && counts)
    {
        cerr << "ERROR: The options -t and -c are incompatible." << endl;
        return 1;
    }
    
    if (dump)
    {
        if (tabular)
        {
            cerr << "ERROR: The options -d and -t are incompatible." << endl;
            return 1;
        }
    
        if (header)
        {
            cerr << "ERROR: The options -d and -H are incompatible." << endl;
            return 1;
        }
    
        if (counts)
        {
            cerr << "ERROR: The options -d and -c are incompatible." << endl;
            return 1;
        }
    }
    
    const string &file = arguments[0];
    
    if (!hasSuffix(file, suffixSketch))
    {
        cerr << "ERROR: The file \"" << file << "\" does not look like a sketch." << endl;
        return 1;
    }
    
    Sketch sketch;
    Sketch::Parameters params;
    params.parallelism = 1;
    
    uint64_t referenceCount;
    
    if (header)
    {
        // Inizializza solo i parametri dello sketch dal file Cap'n Proto
        referenceCount = sketch.initParametersFromCapnp(arguments[0].c_str());
    }
    else if(fingerprint)
    {
        cout << "Va qui !"<< endl;
        // Inizializza lo sketch dai file specificati
        sketch.initFromFingerPrintFiles(arguments, params);
        referenceCount = sketch.getReferenceCount();

    }else{

        // Inizializza lo sketch dai file specificati
        sketch.initFromFiles(arguments, params);
        referenceCount = sketch.getReferenceCount();
    }
    

    cout<< "Sono qua ! dopo l'initFromFingerPrintsFiles() !"<<endl;
    // Gestione delle opzioni specifiche
    if (counts)
    {
        return printCounts(sketch);
    }
    else if (dump)
    {
        cout<<"Poi qui !"<<endl;
        cout << "      \"Write JSON information : " << endl;
        return writeJson(sketch);
    }
    
    if (tabular)
    {
        cout << "#Hashes\tLength\tID\tComment" << endl;
    }
    else
    {
        string alphabet;
        sketch.getAlphabetAsString(alphabet);
        
        cout << "Header:" << endl;
        cout << "  Hash function (seed):          " << HASH << " (" << sketch.getHashSeed() << ")" << endl;
        cout << "  K-mer size:                    " << sketch.getKmerSize() << " (" << (sketch.getUse64() ? "64" : "32") << "-bit hashes)" << endl;
        cout << "  Alphabet:                      " << alphabet 
             << (sketch.getNoncanonical() ? "" : " (canonical)") 
             << (sketch.getPreserveCase() ? " (case-sensitive)" : "") << endl;
        cout << "  Target min-hashes per sketch:  " << sketch.getMinHashesPerWindow() << endl;
        cout << "  Sketches:                      " << referenceCount << endl;
    }

    // Stampa le informazioni degli schizzi
    if (!header)
    {
        vector<vector<string>> columns(4);
        
        if (!tabular)
        {
            cout << endl;
            cout << "Sketches:" << endl;
            columns[0].push_back("[Hashes]");
            columns[1].push_back("[Length]");
            columns[2].push_back("[ID]");
            columns[3].push_back("[Comment]");
        }
        
        for (uint64_t i = 0; i < sketch.getReferenceCount(); i++)
        {
            const Sketch::Reference &ref = sketch.getReference(i);
            
            if (tabular)
            {
                cout << ref.hashesSorted.size() << '\t'
                     << ref.length << '\t'
                     << ref.id<< '\t'
                     << ref.comment << endl;
            }
            else
            {
                columns[0].push_back(std::to_string(ref.hashesSorted.size()));
                columns[1].push_back(std::to_string(ref.length));
                columns[2].push_back(ref.id);
                columns[3].push_back(ref.comment);
            }
        }
        
        if (!tabular)
        {
            printColumns(columns, 2, 2, "-", 0);
        }
    }
    
    return 0;
}

/**
 * Stampa un istogramma delle frequenze degli hash per ogni schizzo.
 * 
 * Verifica che lo sketch contenga dati e calcola l'istogramma dei conteggi degli hash.
 * 
 * @param sketch Lo sketch per cui stampare i conteggi.
 * @return 0 se l'operazione è completata con successo, 1 in caso di errore.
 */
int CommandInfo::printCounts(const Sketch &sketch) const
{
    using std::map;
    
    if (sketch.getReferenceCount() == 0)
    {
        cerr << "ERROR: Sketch file contains no sketches." << endl;
        return 1;
    }
    
    if (!sketch.hasHashCounts())
    {
        cerr << "ERROR: Sketch file does not have hash counts. Re-sketch with -M to use this feature." << endl;
        return 1;
    }
    
    cout << "#Sketch\tBin\tFrequency" << endl;
    
    map<uint32_t, uint64_t> histogram;
    
    for (uint64_t i = 0; i < sketch.getReferenceCount(); i++)
    {
        const string &name = sketch.getReference(i).name;
        
        sketch.getReferenceHistogram(i, histogram);
        
        for (auto &entry : histogram)
        {
            cout << name << '\t' << entry.first << '\t' << entry.second << endl;
        }
    }
    
    return 0;
}

/**
 * Esporta lo sketch in formato JSON.
 * 
 * Serializza i dati dello sketch e li stampa in formato JSON.
 * 
 * @param sketch Lo sketch da esportare.
 * @return 0 se l'operazione è completata con successo, 1 in caso di errore.
 */
int CommandInfo::writeJson(const Sketch &sketch) const
{
    string alphabet;
    sketch.getAlphabetAsString(alphabet);
    bool use64 = sketch.getUse64();
    
    cout << "{" << endl;
    cout << "  \"kmer\" : " << sketch.getKmerSize() << ',' << endl;
    cout << "  \"alphabet\" : \"" << alphabet << "\"," << endl;
    cout << "  \"preserveCase\" : " << (sketch.getPreserveCase() ? "true" : "false") << ',' << endl;
    cout << "  \"canonical\" : " << (sketch.getNoncanonical() ? "false" : "true") << ',' << endl;
    cout << "  \"sketchSize\" : " << sketch.getMinHashesPerWindow() << ',' << endl;
    cout << "  \"hashType\" : \"" << HASH << "\"," << endl;
    cout << "  \"hashBits\" : " << (use64 ? 64 : 32) << ',' << endl;
    cout << "  \"hashSeed\" : " << sketch.getHashSeed() << ',' << endl;
    cout << "  \"sketches\" :" << endl;
    cout << "  [" << endl;
    
    for (uint64_t i = 0; i < sketch.getReferenceCount(); i++){
    
        const Sketch::Reference &ref = sketch.getReference(i);
        
        cout << "    {" << endl;
        cout << "    \"ID\" : \"" << ref.id << "\"," << endl;
        cout << "      \"name\" : \"" << ref.name << "\"," << endl;
        cout << "      \"length\" : " << ref.length << ',' << endl;
        cout << "      \"comment\" : \"" << ref.comment << "\"," << endl;
        cout << "      \"SubSketch\" :" << endl;
        
        cout << "      [" << endl;


        cout << "SIZE SUBSKETCH_LIST : "<< ref.subSketch_list.size();
        
        // Itero su ogni SubSketch 
        for (int j = 0; j < ref.subSketch_list.size(); j++)
        {
            
            const Sketch::SubSketch &subSketch = ref.subSketch_list[j];
            
            cout << "        {" << endl;
            cout << "          \"ID\" : \"" << subSketch.ID << "\"," << endl;
            cout << "          \"hashesSorted\" : [" << endl;
            
            for (size_t k = 0; k < subSketch.hashesSorted.size(); k++)
            {
                cout << "            " << subSketch.hashesSorted.at(k).hash64;
                if (k < subSketch.hashesSorted.size() - 1)
                {
                    cout << ",";
                }
                cout << endl;
            }
            
            cout << "          ]" << endl;
            cout << "        }";
            if (j < ref.subSketch_list.size() - 1)
            {
                cout << ",";
            }
            cout << endl;
        }
        
        cout << "      ]" << endl;
        

        if (ref.countsSorted)
        {
            cout << "      \"counts\" :" << endl;
            cout << "      [" << endl;
        
            for (int j = 0; j < ref.counts.size(); j++)
            {
                cout << "        " << ref.counts.at(j);
                
                if (j < ref.counts.size() - 1)
                {
                    cout << ',';
                }
                
                cout << endl;
            }
        
            cout << "      ]" << endl;
        }
        
        if (i < sketch.getReferenceCount() - 1)
        {
            cout << "    }," << endl;
        }
        else
        {
            cout << "    }" << endl;
        }
    }
    cout << "  ]" << endl;
    cout << "}" << endl;

    return 0;
    }
    
}
