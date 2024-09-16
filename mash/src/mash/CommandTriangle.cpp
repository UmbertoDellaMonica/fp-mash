#include "CommandTriangle.h"
#include "Sketch.h"
#include "sketchParameterSetup.h"
#include <iostream>
#include <zlib.h>
#include "ThreadPool.h"
#include <math.h>

#ifdef USE_BOOST
    #include <boost/math/distributions/binomial.hpp>
    using namespace::boost::math;
#else
    #include <gsl/gsl_cdf.h>
#endif

using namespace::std;

namespace mash {

CommandTriangle::CommandTriangle()
: Command()
{
    name = "triangle";
    summary = "Estimate a lower-triangular distance matrix.";
    description = "Estimate the distance of each input sequence or fingerprint to every other input. Outputs a lower-triangular distance matrix in relaxed Phylip format. The input sequences can be fasta or fastq, gzipped or not, or Mash sketch files (.msh) with matching k-mer sizes. Input files can also be files of file names (see -l). If more than one input file is provided, whole files are compared by default (see -i).";
    argumentString = "<seq1> [<seq2>] ...";
    
    useOption("help");
    addOption("list", Option(Option::Boolean, "l", "Input", "List input. Lines in each <query> specify paths to sequence files, one per line. The reference file is not affected.", ""));
    addOption("comment", Option(Option::Boolean, "C", "Output", "Use comment fields for sequence names instead of IDs.", ""));
    addOption("edge", Option(Option::Boolean, "E", "Output", "Output edge list instead of Phylip matrix, with fields [seq1, seq2, dist, p-val, shared-hashes].", ""));
    addOption("pvalue", Option(Option::Number, "v", "Output", "Maximum p-value to report in edge list. Implies -" + getOption("edge").identifier + ".", "1.0", 0., 1.));
    addOption("distance", Option(Option::Number, "d", "Output", "Maximum distance to report in edge list. Implies -" + getOption("edge").identifier + ".", "1.0", 0., 1.));
    addOption("fingerprint", Option(Option::Boolean, "fp", "Input", "Indicates that the input files are fingerprints instead of sequences.", "")); // Aggiunto
    useSketchOptions();
}

int CommandTriangle::run() const
{
    if (arguments.size() < 1 || options.at("help").active)
    {
        print();
        return 0;
    }

    int threads = options.at("threads").getArgumentAsNumber();
    bool list = options.at("list").active;
    bool comment = options.at("comment").active;
    bool edge = options.at("edge").active;
    bool fingerprint = options.at("fingerprint").active; // Aggiunto
    double pValueMax = options.at("pvalue").getArgumentAsNumber();
    double distanceMax = options.at("distance").getArgumentAsNumber();
    double pValuePeakToSet = 0;

    if (options.at("pvalue").active || options.at("distance").active)
    {
        edge = true;
    }

    Sketch::Parameters parameters;
    
    if ( sketchParameterSetup(parameters, *(Command *)this) )
    {
    	return 1;
    }
    
    if ( arguments.size() == 1 && !list )
    {
    	parameters.concatenated = false;
    }

    Sketch sketch;
    
    // Vecchie variabili reintegrate
    uint64_t lengthMax;
    double randomChance;
    int kMin;
    string lengthMaxName;
    int warningCount = 0;
    
    vector<string> queryFiles;

    for (int i = 0; i < arguments.size(); i++)
    {
        if (list)
        {
            splitFile(arguments[i], queryFiles);
        }
        else
        {
            queryFiles.push_back(arguments[i]);
        }
    }

    // TAG = Format File .msh 

    /*
        Se il file ha il formato .msh  : ( Inserisco l'opzione -fp )
        - Devo utilizzare initFromFiles()

        Se il file ha il formato .txt : ( Inserisco l'opzione -fp )
        - Posso utilizzare initFromFingerPrint()   

    */

   /**
    *  FingerPrint Option & Msh
    */
   if(fingerprint && containsExtensionMSH(queryFiles)){
        
        sketch.initFromFiles(queryFiles,parameters);
   }
   /**
    *  FingerPrint Option & txt
    */   
   else if( fingerprint && containsExtensionTXT(queryFiles)){
        
        sketch.initFromFingerprints(queryFiles,parameters);
   }
    else if (fingerprint) {
        sketch.initFromFingerprints(queryFiles, parameters); // Caricamento fingerprint
    } else {
        sketch.initFromFiles(queryFiles, parameters); // Caricamento sequenze genomiche
    }

    double lengthThreshold = (parameters.warning * sketch.getKmerSpace()) / (1. - parameters.warning);

    for (uint64_t i = 0; i < sketch.getReferenceCount(); i++)
    {
        uint64_t length = sketch.getReference(i).length;

        if (length > lengthThreshold)
        {
            if (warningCount == 0 || length > lengthMax)
            {
                lengthMax = length;
                lengthMaxName = sketch.getReference(i).name;
                randomChance = sketch.getRandomKmerChance(i);
                kMin = sketch.getMinKmerSize(i);
            }

            warningCount++;
        }
    }
    if (!edge)
    {
        cout << '\t' << sketch.getReferenceCount() << endl;
        cout << (comment ? sketch.getReference(0).comment : sketch.getReference(0).name) << endl;
    }

    ThreadPool<TriangleInput, TriangleOutput> threadPool(compare, threads);

    for (uint64_t i = 1; i < sketch.getReferenceCount(); i++)
    {
        threadPool.runWhenThreadAvailable(new TriangleInput(sketch, i, parameters, distanceMax, pValueMax, fingerprint)); // Passaggio del parametro fingerprint
        while (threadPool.outputAvailable())
        {
            writeOutput(threadPool.popOutputWhenAvailable(), comment, edge, pValuePeakToSet);
        }
    }

    while (threadPool.running())
    {
        writeOutput(threadPool.popOutputWhenAvailable(), comment, edge, pValuePeakToSet);
    }

    if (!edge)
    {
        cerr << "Max p-value: " << pValuePeakToSet << endl;
    }

    if (warningCount > 0 && !parameters.reads)
    {
        warnKmerSize(parameters, *this, lengthMax, lengthMaxName, randomChance, kMin, warningCount);
    }

    return 0;
}

bool containsExtensionMSH(const std::vector<std::string>& strVec) {
    
    bool flag = false;

    for (const auto& str : strVec) {
         flag = str.find(".msh") != std::string::npos;
    }

    return flag;

}



bool containsExtensionTXT(const std::vector<std::string>& strVec){
    bool flag = false;

    for (const auto& str : strVec) {
         flag = str.find(".txt") != std::string::npos;
    }

    return flag;
}



void CommandTriangle::writeOutput(TriangleOutput * output, bool comment, bool edge, double & pValuePeakToSet) const
{
    const Sketch & sketch = output->sketch;
    const Sketch::Reference & ref = sketch.getReference(output->index);
    
    if (!edge)
    {
        cout << (comment ? ref.comment : ref.name);
    }
    
    for (uint64_t i = 0; i < output->index; i++)
    {
        const CommandDistance::CompareOutput::PairOutput * pair = &output->pairs[i];
        
        if (edge)
        {
            if (pair->pass)
            {
                const Sketch::Reference & qry = sketch.getReference(i);
                cout << (comment ? ref.comment : ref.name) << '\t' << (comment ? qry.comment : qry.name) << '\t' << pair->distance << '\t' << pair->pValue << '\t' << pair->numer << '/' << pair->denom << endl;
            }
        }
        else
        {
            cout << '\t' << pair->distance;
        }
        
        if (pair->pValue > pValuePeakToSet)
        {
            pValuePeakToSet = pair->pValue;
        }
    }
    
    if (!edge)
    {
        cout << endl;
    }
    
    delete output;
}

CommandTriangle::TriangleOutput * compare(CommandTriangle::TriangleInput * input)
{
    const Sketch & sketch = input->sketch;
    CommandTriangle::TriangleOutput * output = new CommandTriangle::TriangleOutput(input->sketch, input->index);
    uint64_t sketchSize = sketch.getMinHashesPerWindow();

    for (uint64_t i = 0; i < input->index; i++)
    {
        if (input->isFingerprint) {
            compareFingerprints(&output->pairs[i], sketch.getReference(input->index), sketch.getReference(i), sketchSize, input->maxDistance, input->maxPValue); // Nuovo confronto fingerprint
        } else {
            compareSketches(&output->pairs[i], sketch.getReference(input->index), sketch.getReference(i), sketchSize, sketch.getKmerSize(), sketch.getKmerSpace(), input->maxDistance, input->maxPValue);
        }
    }

    return output;
}

void compareFingerprints(CommandDistance::CompareOutput::PairOutput * pair, const Sketch::Reference & ref1, const Sketch::Reference & ref2, uint64_t sketchSize, double maxDistance, double maxPValue)
{
    int matches = 0;

    int size1 = ref1.hashesSorted.size();
    int size2 = ref2.hashesSorted.size();
    int minSize = std::min(size1, size2);

    for (int i = 0; i < minSize; i++)
    {
        hash_u hash1 = ref1.hashesSorted.at(i);
        hash_u hash2 = ref2.hashesSorted.at(i);

        // Determiniamo se stiamo usando hash a 64-bit o a 32-bit basandoci sulla dimensione del valore restituito
        if (hash1.hash64 != 0 || hash2.hash64 != 0) // Supponiamo che se `hash64` è diverso da 0, stiamo usando `hashes64`
        {
            if (hash1.hash64 == hash2.hash64)
            {
                matches++;
            }
        }
        else // Altrimenti stiamo usando `hashes32`
        {
            if (hash1.hash32 == hash2.hash32)
            {
                matches++;
            }
        }
    }

    // Calcola la distanza come 1 - proporzione di hash corrispondenti
    pair->distance = 1.0 - (double(matches) / minSize);
    pair->pValue = gsl_cdf_chisq_Q(matches, 1); // Esempio di calcolo p-value

    pair->pass = (pair->distance <= maxDistance && pair->pValue <= maxPValue);
    pair->numer = matches;
    pair->denom = minSize;
}

} // namespace mash
