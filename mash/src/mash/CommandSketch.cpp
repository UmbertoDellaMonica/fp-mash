// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "CommandSketch.h"
#include "Sketch.h"
#include "sketchParameterSetup.h"
#include <iostream>

using std::cerr;
using std::endl;
using std::string;
using std::vector;

namespace mash {

CommandSketch::CommandSketch()
: Command()
{
    name = "sketch";
    summary = "Create sketches (reduced representations for fast operations).";
    description = "Create a sketch file, which is a reduced representation of a sequence or set of sequences (based on min-hashes) that can be used for fast distance estimations. Inputs can be fasta or fastq files (gzipped or not), and \"-\" can be given to read from standard input. Input files can also be files of file names (see -l). For output, one sketch file will be generated, but it can have multiple sketches within it, divided by sequences or files (see -i). By default, the output file name will be the first input file with a '.msh' extension, or 'stdin.msh' if standard input is used (see -o).";
    argumentString = "<input> [<input>] ...";
    
    useOption("help");
    addOption("list", Option(Option::Boolean, "l", "Input", "List input. Lines in each <input> specify paths to sequence files, one per line.", ""));
    addOption("prefix", Option(Option::File, "o", "Output", "Output prefix (first input file used if unspecified). The suffix '.msh' will be appended.", ""));
    addOption("id", Option(Option::File, "I", "Sketch", "ID field for sketch of reads (instead of first sequence ID).", ""));
    addOption("comment", Option(Option::File, "C", "Sketch", "Comment for a sketch of reads (instead of first sequence comment).", ""));
    addOption("counts", Option(Option::Boolean, "M", "Sketch", "Store multiplicity of each k-mer in each sketch.", ""));
    addOption("fingerprint", Option(Option::Boolean, "fp", "Input", "Indicates that the input files are fingerprints instead of sequences.", "")); // Aggiunto
    useSketchOptions();
}

int CommandSketch::run() const
{
    if (arguments.size() == 0 || options.at("help").active)
    {
        print();
        return 0;
    }

    int verbosity = 1; // options.at("silent").active ? 0 : options.at("verbose").active ? 2 : 1;
    bool list = options.at("list").active;
    bool fingerprint = options.at("fingerprint").active; // Nuova opzione

    Sketch::Parameters parameters;
    parameters.counts = options.at("counts").active;

    if (sketchParameterSetup(parameters, *(Command *)this))
    {
        return 1;
    }

    vector<string> files;
    for (int i = 0; i < arguments.size(); i++)
    {
        if (list)
        {
            splitFile(arguments[i], files);
        }
        else
        {
            files.push_back(arguments[i]);
        }
    }

    Sketch sketch;

    if (parameters.reads)
    {
        sketch.initFromReads(files, parameters);
    }
    else if (fingerprint)
    {
        sketch.initFromFingerprints(files, parameters); // Nuova funzione per fingerprint
    }
    else
    {
        sketch.initFromFiles(files, parameters, verbosity);
    }

    if (getOption("id").active)
    {
        sketch.setReferenceName(0, getOption("id").argument);
    }

    if (getOption("comment").active)
    {
        sketch.setReferenceComment(0, getOption("comment").argument);
    }

    string prefix;
    if (options.at("prefix").argument.length() > 0)
    {
        prefix = options.at("prefix").argument;
    }
    else
    {
        if (arguments[0] == "-")
        {
            prefix = "stdin";
        }
        else
        {
            prefix = arguments[0];
        }
    }

    string suffix = parameters.windowed ? suffixSketchWindowed : suffixSketch;
    if (!hasSuffix(prefix, suffix))
    {
        prefix += suffix;
    }

    cerr << "Writing to " << prefix << "..." << endl;
    sketch.writeToCapnp(prefix.c_str());

    return 0;
}

} // namespace mash