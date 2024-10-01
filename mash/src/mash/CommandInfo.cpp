// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "CommandInfo.h"
#include "SketchFingerPrint.h"
#include "Sketch.h"
#include <iostream>
#include <sstream>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;

namespace mash
{

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
        addOption("fingerprint", Option(Option::Boolean, "fp", "",
                                        "Dump fingerprint sketches in JSON format. Incompatibile with -H , -t , -c.", ""));
    }

    int CommandInfo::run() const
    {
        // Verifica se ci sono gli argomenti giusti
        if (!checkArguments())
        {
            return 1; // Ritorna un errore se i parametri non sono validi
        }

        bool fingerprint = options.at("fingerprint").active;

        if (fingerprint)
        {
            return runFingerPrint();
        }

        bool header = options.at("header").active;
        bool tabular = options.at("tabular").active;
        bool counts = options.at("counts").active;
        bool dump = options.at("dump").active;

        if (checkHeader(header, tabular, counts, dump) == 1)
        {

            return 1;
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
            referenceCount = sketch.initParametersFromCapnp(arguments[0].c_str());
        }
        else
        {
            sketch.initFromFiles(arguments, params);
            referenceCount = sketch.getReferenceCount();
        }

        if (counts)
        {
            return printCounts(sketch);
        }
        else if (dump)
        {
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
            cout << "  Alphabet:                      " << alphabet << (sketch.getNoncanonical() ? "" : " (canonical)") << (sketch.getPreserveCase() ? " (case-sensitive)" : "") << endl;
            cout << "  Target min-hashes per sketch:  " << sketch.getMinHashesPerWindow() << endl;
            cout << "  Sketches:                      " << referenceCount << endl;
        }

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
                    cout
                        << ref.hashesSorted.size() << '\t'
                        << ref.length << '\t'
                        << ref.name << '\t'
                        << ref.comment << endl;
                }
                else
                {
                    columns[0].push_back(std::to_string(ref.hashesSorted.size()));
                    columns[1].push_back(std::to_string(ref.length));
                    columns[2].push_back(ref.name);
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

    int CommandInfo::runFingerPrint() const
    {

        bool header = options.at("header").active;
        bool tabular = options.at("tabular").active;
        bool counts = options.at("counts").active;
        bool dump = options.at("dump").active;

        if (checkHeader(header, tabular, counts, dump) == 1)
        {

            return 1;
        }

        const string &file = arguments[0];

        if (!hasSuffixFingerPrint(file, suffixFingerPrintSketch))
        {
            cerr << "ERROR: The file \"" << file << "\" does not look like a sketch." << endl;
            return 1;
        }

        SketchFingerPrint sketchFingerprint;
        SketchFingerPrint::Parameters paramsFingerPrint;

        paramsFingerPrint.parallelism = 1;

        uint64_t referenceCount;

        // Inizializza lo sketch dai file specificati
        sketchFingerprint.initFromFingerPrintFiles(arguments, paramsFingerPrint);
        referenceCount = sketchFingerprint.getReferenceCount();

        // Gestione delle opzioni specifiche
        if (counts)
        {
            return printFingerPrintCounts(sketchFingerprint);
        }
        else if (dump)
        {
            return writeFingerPrintJson(sketchFingerprint);
        }

        if (tabular)
        {
            cout << "#Hashes\tLength\tID\tComment" << endl;
        }
        else
        {
            string alphabet;
            sketchFingerprint.getAlphabetAsString(alphabet);

            cout << "Header:" << endl;
            cout << "  Hash function (seed):          " << HASH << " (" << sketchFingerprint.getHashSeed() << ")" << endl;
            cout << "  K-mer size:                    " << sketchFingerprint.getKmerSize() << " (" << (sketchFingerprint.getUse64() ? "64" : "32") << "-bit hashes)" << endl;
            cout << "  Alphabet:                      " << alphabet
                 << (sketchFingerprint.getNoncanonical() ? "" : " (canonical)")
                 << (sketchFingerprint.getPreserveCase() ? " (case-sensitive)" : "") << endl;
            cout << "  Target min-hashes per sketch:  " << sketchFingerprint.getMinHashesPerWindow() << endl;
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
                columns[2].push_back("[Comment]");
            }

            for (uint64_t i = 0; i < sketchFingerprint.getReferenceCount(); i++)
            {
                const SketchFingerPrint::Reference &ref = sketchFingerprint.getReference(i);

                if (tabular)
                {
                    cout << ref.subSketch_list.size() << '\t'
                         << ref.length << '\t'
                         << ref.comment << endl;
                }
                else
                {
                    columns[0].push_back(std::to_string(ref.subSketch_list.size()));
                    columns[1].push_back(std::to_string(ref.length));
                    columns[2].push_back(ref.comment);
                }
            }

            if (!tabular)
            {
                printColumns(columns, 2, 2, "-", 0);
            }
        }

        return 0;
    }

    int CommandInfo::checkHeader(bool header, bool tabular, bool counts, bool dump) const {

        /**
         * @brief Verifica la compatibilità delle opzioni di intestazione.
         *
         * Questo metodo controlla se ci sono opzioni incompatibili tra di loro.
         * Le opzioni considerate sono:
         * - `header` (-H): Stampa l'intestazione.
         * - `tabular` (-t): Stampa in formato tabulare.
         * - `counts` (-c): Stampa i conteggi.
         * - `dump` (-d): Stampa un dump dei dati.
         *
         * La funzione restituisce 1 se ci sono opzioni incompatibili e visualizza
         * un messaggio di errore su `cerr`, altrimenti restituisce 0.
         *
         * Le combinazioni incompatibili sono:
         * - Se `header` e `tabular` sono entrambi attivi.
         * - Se `header` e `counts` sono entrambi attivi.
         * - Se `tabular` e `counts` sono entrambi attivi.
         * - Se `dump` è attivo, non può essere attivo nessuna delle altre opzioni.
         *
         * @param header  Booleano che indica se l'opzione -H è attiva.
         * @param tabular Booleano che indica se l'opzione -t è attiva.
         * @param counts  Booleano che indica se l'opzione -c è attiva.
         * @param dump    Booleano che indica se l'opzione -d è attiva.
         * @return 0 se le opzioni sono compatibili, 1 in caso contrario.
         */

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

        return 0;
    }

    bool CommandInfo::checkArguments() const{
        /**
         * @brief Verifica la validità degli argomenti passati al comando.
         *
         * Questo metodo controlla se la lista degli argomenti è vuota o se l'opzione "help" è attiva.
         * Se uno di questi casi è vero, il metodo chiama la funzione `print` per visualizzare
         * un messaggio di aiuto e ritorna `false`, indicando che gli argomenti non sono validi.
         *
         * @return `true` se gli argomenti sono validi e il comando può continuare; `false` altrimenti.
         *
         * La verifica consiste nei seguenti passaggi:
         * 1. Controlla se la lista degli argomenti (`arguments`) è vuota.
         * 2. Controlla se l'opzione "help" (`options.at("help")`) è attiva.
         *    - `options.at("help").active` verifica se l'opzione "help" è stata specificata.
         *
         * Se uno dei due controlli è positivo, il metodo:
         * 1. Chiama la funzione `print()` per visualizzare il messaggio di aiuto.
         * 2. Ritorna `false` per indicare che la verifica degli argomenti è fallita.
         *
         * Se entrambi i controlli sono negativi, il metodo ritorna `true` per indicare che gli argomenti
         * sono validi e il comando può proseguire con l'esecuzione.
         *
         * Questo metodo è privato e viene utilizzato internamente nella classe `CommandSketch` per
         * assicurare che gli argomenti siano appropriati prima di eseguire il comando.
         */
        if (arguments.size() != 1 || options.at("help").active)
        {
            print();
            return false;
        }
        return true;
    }


    /**
     * Stampa un istogramma delle frequenze degli hash per ogni schizzo.
     *
     * Verifica che lo sketch contenga dati e calcola l'istogramma dei conteggi degli hash.
     *
     * @param sketch Lo sketch per cui stampare i conteggi.
     * @return 0 se l'operazione è completata con successo, 1 in caso di errore.
     */
    int CommandInfo::printFingerPrintCounts(const SketchFingerPrint &sketch) const
    {

        /**
         * @brief Stampa i conteggi degli hash delle referenze in un oggetto SketchFingerPrint.
         *
         * Questo metodo verifica se l'oggetto `sketch` contiene referenze e conteggi hash.
         * Se non ci sono referenze o se non ci sono conteggi hash, vengono visualizzati
         * messaggi di errore su `cerr`.
         *
         * In caso di successo, il metodo stampa le intestazioni delle colonne e le frequenze
         * per ciascuna referenza nel formato:
         * ```
         * #Sketch   Bin   Frequency
         * ```
         * Dove `#Sketch` è il nome della referenza, `Bin` è il valore del bin e
         * `Frequency` è il conteggio associato.
         *
         * @param sketch Riferimento a un oggetto `SketchFingerPrint` contenente i dati da analizzare.
         * @return 0 se l'operazione ha avuto successo, 1 se ci sono stati errori.
         */
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
    
    int CommandInfo::printCounts(const Sketch &sketch) const
    {
        using std::map;

        if (sketch.getReferenceCount() == 0)
        {
            cerr << "ERROR: Sketch file contains no sketches" << endl;
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

            for (map<uint32_t, uint64_t>::const_iterator j = histogram.begin(); j != histogram.end(); j++)
            {
                cout << name << '\t' << j->first << '\t' << j->second << endl;
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
        cout << "	\"kmer\" : " << sketch.getKmerSize() << ',' << endl;
        cout << "	\"alphabet\" : \"" << alphabet << "\"," << endl;
        cout << "	\"preserveCase\" : " << (sketch.getPreserveCase() ? "true" : "false") << ',' << endl;
        cout << "	\"canonical\" : " << (sketch.getNoncanonical() ? "false" : "true") << ',' << endl;
        cout << "	\"sketchSize\" : " << sketch.getMinHashesPerWindow() << ',' << endl;
        cout << "	\"hashType\" : \"" << HASH << "\"," << endl;
        cout << "	\"hashBits\" : " << (use64 ? 64 : 32) << ',' << endl;
        cout << "	\"hashSeed\" : " << sketch.getHashSeed() << ',' << endl;
        cout << " 	\"sketches\" :" << endl;
        cout << "	[" << endl;

        for (uint64_t i = 0; i < sketch.getReferenceCount(); i++)
        {
            const Sketch::Reference &ref = sketch.getReference(i);

            cout << "		{" << endl;
            cout << "			\"name\" : \"" << ref.name << "\"," << endl;
            cout << "			\"length\" : " << ref.length << ',' << endl;
            cout << "			\"comment\" : \"" << ref.comment << "\"," << endl;
            cout << "			\"hashes\" :" << endl;
            cout << "			[" << endl;

            for (int j = 0; j < ref.hashesSorted.size(); j++)
            {
                cout << "				" << (use64 ? ref.hashesSorted.at(j).hash64 : ref.hashesSorted.at(j).hash32);

                if (j < ref.hashesSorted.size() - 1)
                {
                    cout << ',';
                }

                cout << endl;
            }

            cout << "			]" << endl;

            if (ref.countsSorted)
            {
                cout << "			\"counts\" :" << endl;
                cout << "			[" << endl;

                for (int j = 0; j < ref.counts.size(); j++)
                {
                    cout << "				" << ref.counts.at(j);

                    if (j < ref.hashesSorted.size() - 1)
                    {
                        cout << ',';
                    }

                    cout << endl;
                }

                cout << "			]" << endl;
            }

            if (i < sketch.getReferenceCount() - 1)
            {
                cout << "		}," << endl;
            }
            else
            {
                cout << "		}" << endl;
            }
        }

        cout << "	]" << endl;
        cout << "}" << endl;

        return 0;
    }

    int CommandInfo::writeFingerPrintJson(const SketchFingerPrint &sketch) const
    {

        std::ostringstream oss;
        std::string alphabet;
        sketch.getAlphabetAsString(alphabet);
        bool use64 = sketch.getUse64();

        oss << "{" << std::endl;
        oss << "  \"kmer\" : " << sketch.getKmerSize() << ',' << std::endl;
        oss << "  \"alphabet\" : \"" << alphabet << "\"," << std::endl;
        oss << "  \"preserveCase\" : " << (sketch.getPreserveCase() ? "true" : "false") << ',' << std::endl;
        oss << "  \"canonical\" : " << (sketch.getNoncanonical() ? "false" : "true") << ',' << std::endl;
        oss << "  \"sketchSize\" : " << sketch.getMinHashesPerWindow() << ',' << std::endl;
        oss << "  \"hashType\" : \"" << HASH << "\"," << std::endl;
        oss << "  \"hashBits\" : " << (use64 ? 64 : 32) << ',' << std::endl;
        oss << "  \"hashSeed\" : " << sketch.getHashSeed() << ',' << std::endl;
        oss << "  \"sketches\" :" << std::endl;
        oss << "  [" << std::endl;

        for (uint64_t i = 0; i < sketch.getReferenceCount(); ++i)
        {
            const auto &ref = sketch.getReference(i);

            oss << "    {" << std::endl;
            oss << "      \"ID\" : \"" << ref.id << "\"," << std::endl;
            oss << "      \"name\" : \"" << ref.name << "\"," << std::endl;
            oss << "      \"length\" : " << ref.length << ',' << std::endl;
            oss << "      \"comment\" : \"" << ref.comment << "\"," << std::endl;
            oss << "      \"Hashes\" : {" << std::endl;

            for (size_t j = 0; j < ref.subSketch_list.size(); ++j)
            {
                const auto &subsketch_hashlist = ref.subSketch_list.at(j);

                oss << "        [";

                for (size_t k = 0; k < subsketch_hashlist.size(); ++k)
                {
                    if (use64)
                    {
                        oss << subsketch_hashlist.at(k).hash64;
                    }
                    else
                    {
                        oss << subsketch_hashlist.at(k).hash32;
                    }

                    if (k < subsketch_hashlist.size() - 1)
                    {
                        oss << ", ";
                    }
                }

                oss << "]";
                if (j < ref.subSketch_list.size() - 1)
                {
                    oss << ",";
                }
                oss << std::endl;
            }

            oss << "      }" << std::endl;
            oss << "    }";
            if (i < sketch.getReferenceCount() - 1)
            {
                oss << ",";
            }
            oss << std::endl;
        }
        oss << "  ]" << std::endl;
        oss << "}" << std::endl;

        std::cout << oss.str();

        return 0;
    }



} // namespace mash
