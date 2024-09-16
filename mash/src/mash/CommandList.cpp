// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include <iostream>
#include "version.h"
#include "license.h"
#include "CommandList.h"
#include <string.h>



using std::cout;
using std::endl;
using std::string;

namespace mash {


// Costruttore della classe CommandList
// Inizializza il nome del comando con il valore fornito
CommandList::CommandList(string nameNew){
    
    name = nameNew;
}

// Distruttore della classe CommandList
// Dealloca la memoria per tutti i comandi memorizzati nella mappa
CommandList::~CommandList() {
    // Itera su tutti gli elementi della mappa 'commands'
    for (auto& commandPair : commands) {
        // Libera la memoria allocata per il comando
        delete commandPair.second;
    }
}

// Aggiunge un nuovo comando alla lista dei comandi
// Il comando viene memorizzato nella mappa 'commands' con il suo nome come chiave
void CommandList::addCommand(Command* command) {
    // Inserisce il comando nella mappa, utilizzando il nome del comando come chiave
    commands[command->name] = command;
}


// Stampa le informazioni sulla versione e l'uso di Mash, 
// insieme alla lista dei comandi disponibili e le loro descrizioni
void CommandList::print() {
    using std::map;
    using std::vector;

    // Crea una struttura per memorizzare le colonne da stampare
    vector<vector<string>> columns(1);

    // Stampa la versione del software
    cout << endl << "Mash version " << SOFTWARE_VERSION << endl << endl;
    cout << "Type 'mash --license' for license and copyright information." << endl << endl;

    // Stampa le istruzioni generali sull'uso
    cout << "Usage:" << endl << endl;
    columns[0].push_back(name + " <command> [options] [arguments ...]");
    printColumns(columns);

    // Stampa l'intestazione per la sezione dei comandi
    cout << "Commands:" << endl << endl;

    // Trova la lunghezza massima del nome dei comandi per formattare l'output
    int lengthMax = 0;
    for (const auto& commandPair : commands) {
        lengthMax = std::max(lengthMax, static_cast<int>(commandPair.first.length()));
    }

    // Prepara le colonne per la stampa dei comandi e delle loro descrizioni
    columns.clear();
    columns.resize(2);

    // Aggiunge i comandi e le loro descrizioni alle colonne
    for (const auto& commandPair : commands) {
        columns[0].push_back(commandPair.first);
        columns[1].push_back(commandPair.second->summary);
    }

    // Stampa le colonne dei comandi
    printColumns(columns);
}

int CommandList::run(int argc, const char** argv) {
    // Check if the user requested the version
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        cout << SOFTWARE_VERSION << endl;
        return 0;
    }

    // Check if the user requested the license information
    if (argc > 1 && strcmp(argv[1], "--license") == 0) {
        showLicense();
        return 0;
    }

    // If no command is provided or the command is not found, display the usage information
    if (argc < 2 || commands.find(argv[1]) == commands.end()) {
        print();
        return 0;
    }

    // Execute the command with its associated options and arguments
    return commands.at(argv[1])->run(argc - 2, argv + 2);
}


void CommandList::showLicense() {
    // Stampa lo scopo del software e le informazioni sulla sua distribuzione
    cout << PURPOSE << endl;

#ifdef DIST_LICENSE
    // Stampa informazioni aggiuntive sulla licenza se DIST_LICENSE è definito
    cout << ADDITIONAL_LICENSE_INFO << endl;
#endif

    // Stampa il copyright e le condizioni di licenza
    cout << COPYRIGHT_LICENSE << endl;

#ifdef DIST_LICENSE
    // Stampa le licenze Boost e MIT se DIST_LICENSE è definito
    cout << license::BOOST_LICENSE << endl;
    cout << license::MIT_LICENSE << endl;
#else
    // Stampa solo la licenza MIT se DIST_LICENSE non è definito
    cout << MIT_LICENSE << endl;
#endif
}

} // namespace mash
