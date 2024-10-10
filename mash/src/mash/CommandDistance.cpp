#include "CommandDistance.h"
#include "Sketch.h"
#include "SketchFingerPrint.h"
#include "HashList.h"
#include <iostream>
#include <zlib.h>
#include "ThreadPool.h"
#include "sketchParameterSetup.h"
#include <math.h>
#include <unordered_set>
#include <iomanip>


#ifdef USE_BOOST
    #include <boost/math/distributions/binomial.hpp>
    using namespace::boost::math;
#else
    #include <gsl/gsl_cdf.h>
#endif

using namespace::std;

#define THRESHOLD 0.25


namespace mash {

CommandDistance::CommandDistance()
: Command()
{
    name = "dist";
    summary = "Estimate the distance of query sequences to references.";
    description = "Estimate the distance of each query sequence to the reference. Both the reference and queries can be fasta or fastq, gzipped or not, or Mash sketch files (.msh) with matching k-mer sizes. Query files can also be files of file names (see -l). Whole files are compared by default (see -i). The output fields are [reference-ID, query-ID, distance, p-value, shared-hashes].";
    argumentString = "<reference> <query> [<query>] ...";
    
    useOption("help");
    addOption("list", Option(Option::Boolean, "l", "Input", "List input. Lines in each <query> specify paths to sequence files, one per line. The reference file is not affected.", ""));
    addOption("table", Option(Option::Boolean, "t", "Output", "Table output (will not report p-values, but fields will be blank if they do not meet the p-value threshold).", ""));
    addOption("pvalue", Option(Option::Number, "v", "Output", "Maximum p-value to report.", "1.0", 0., 1.));
    addOption("distance", Option(Option::Number, "d", "Output", "Maximum distance to report.", "1.0", 0., 1.));
    addOption("comment", Option(Option::Boolean, "C", "Output", "Show comment fields with reference/query names (denoted with ':').", "1.0", 0., 1.));
    addOption("fingerprint", Option(Option::Boolean, "fp", "Input", "Indicates that the input files are fingerprints instead of sequences.", "")); // Aggiunto
    addOption("exact-similarity", Option(Option::Boolean, "es", "Input", "Indicates which algorithm use to calculate the similarity of Jaccard. - 1) Calculate Exactly Jaccard Similarity ", "")); // Aggiunto
    addOption("percentage-similarity", Option(Option::Boolean, "ps", "Input", "Indicates which algorithm use to calculate the similarity of Jaccard. Calculate Percentage Jaccard Similarity   ", "")); // Aggiunto
    useSketchOptions();
}

int CommandDistance::run() const
{


    bool fingerprint = options.at("fingerprint").active; // Nuova opzione

    if(fingerprint){
        return runFingerPrint();
    }

    if ( arguments.size() < 2 || options.at("help").active )
    {
        print();
        return 0;
    }
    
    int threads = options.at("threads").getArgumentAsNumber();
    bool list = options.at("list").active;
    bool table = options.at("table").active;
    bool comment = options.at("comment").active;
    double pValueMax = options.at("pvalue").getArgumentAsNumber();
    double distanceMax = options.at("distance").getArgumentAsNumber();


    
    Sketch::Parameters parameters;
    
    if ( sketchParameterSetup(parameters, *(Command *)this) )
    {
    	return 1;
    }
    
    Sketch sketchRef;
    SketchFingerPrint SketchFingerPrintRef;
    
    uint64_t lengthMax;
    double randomChance;
    int kMin;
    string lengthMaxName;
    int warningCount = 0;
    
    const string & fileReference = arguments[0];
    
    bool isSketch = hasSuffix(fileReference, suffixSketch);
    
    if ( isSketch )
    {
        if ( options.at("kmer").active )
        {
            cerr << "ERROR: The option -" << options.at("kmer").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        
        if ( options.at("noncanonical").active )
        {
            cerr << "ERROR: The option -" << options.at("noncanonical").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        
        if ( options.at("protein").active )
        {
            cerr << "ERROR: The option -" << options.at("protein").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        
        if ( options.at("alphabet").active )
        {
            cerr << "ERROR: The option -" << options.at("alphabet").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        // Inserimento del metodo del calcolo non attitente al comando Mash normale ma solo per le fingerprint
        if( options.at("percentage-similarity").active){
            
            cerr << "ERROR: The option -" << options.at("percentage-similarity").identifier << " cannot be used when a simple sketch is provided; it is only for fingerprint-sketch." << endl;
            return 1;
        }
        // Inserimento del metodo del calcolo non attitente al comando Mash normale ma solo per le fingerprint
        if( options.at("exact-similarity").active){
            
            cerr << "ERROR: The option -" << options.at("exact-similarity").identifier << " cannot be used when a simple sketch is provided; it is only for fingerprint-sketch." << endl;
            return 1;
        }
    }
    else
    {
        cerr << "Sketching " << fileReference << " (provide sketch file made with \"mash sketch\" to skip)...";
    }
    
    vector<string> refArgVector;
    refArgVector.push_back(fileReference);

    /**
     * Se utilizzo l'opzione di fingerprint voglio che i due file che devo testare sono : 
     * - .msh e .msh oppure .txt e .txt 
     * 
     * - In questo caso se utilizzo -fp con .txt .txt -> utilizzo la funzione .initFromFingerprints();
     * 
     * - Altrimenti se utilizzo -fp con .msh .msh -> utilizzo la funzione .initFromFiles(); 
     */
    
    sketchRef.initFromFiles(refArgVector, parameters);
    
    double lengthThreshold = (parameters.warning * sketchRef.getKmerSpace()) / (1. - parameters.warning);
    
    if ( isSketch )
    {
        if ( options.at("sketchSize").active )
        {
            if ( parameters.reads && parameters.minHashesPerWindow != sketchRef.getMinHashesPerWindow() )
            {
                cerr << "ERROR: The sketch size must match the reference when using a bloom filter (leave this option out to inherit from the reference sketch)." << endl;
                return 1;
            }
        }
        
        parameters.minHashesPerWindow = sketchRef.getMinHashesPerWindow();
        parameters.kmerSize = sketchRef.getKmerSize();
        parameters.noncanonical = sketchRef.getNoncanonical();
        parameters.preserveCase = sketchRef.getPreserveCase();
        parameters.seed = sketchRef.getHashSeed();
        
        string alphabet;
        sketchRef.getAlphabetAsString(alphabet);
        setAlphabetFromString(parameters, alphabet.c_str());
    }
    else
    {
        for ( uint64_t i = 0; i < sketchRef.getReferenceCount(); i++ )
        {
            uint64_t length = sketchRef.getReference(i).length;
        
            if ( length > lengthThreshold )
            {
                if ( warningCount == 0 || length > lengthMax )
                {
                    lengthMax = length;
                    lengthMaxName = sketchRef.getReference(i).name;
                    randomChance = sketchRef.getRandomKmerChance(i);
                    kMin = sketchRef.getMinKmerSize(i);
                }
            
                warningCount++;
            }
        }
    
        cerr << "done.\n";
    }
    
    if ( table )
    {
        cout << "#query";
        
        for ( int i = 0; i < sketchRef.getReferenceCount(); i++ )
        {
            cout << '\t' << sketchRef.getReference(i).name;
        }
        
        cout << endl;
    }
    

    ThreadPool<CompareInput, CompareOutput> threadPool(compare, threads);
    
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
    
    Sketch sketchQuery;
    

    sketchQuery.initFromFiles(queryFiles, parameters, 0, true);
    
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
        
        threadPool.runWhenThreadAvailable(new CompareInput(sketchRef, sketchQuery, j, i, pairsPerThread, parameters, distanceMax, pValueMax));
        
        while ( threadPool.outputAvailable() )
        {
            writeOutput(threadPool.popOutputWhenAvailable(), table, comment);
        }
    }
    
    while ( threadPool.running() )
    {
        writeOutput(threadPool.popOutputWhenAvailable(), table, comment);
    }
    
    if ( warningCount > 0 && ! parameters.reads )
    {
    	warnKmerSize(parameters, *this, lengthMax, lengthMaxName, randomChance, kMin, warningCount);
    }
    
    return 0;
}


int CommandDistance::runFingerPrint() const{

    if ( arguments.size() < 2 || options.at("help").active )
    {
        print();
        return 0;
    }
    
    int threads = options.at("threads").getArgumentAsNumber();
    bool list = options.at("list").active;
    bool table = options.at("table").active;
    bool comment = options.at("comment").active;
    double pValueMax = options.at("pvalue").getArgumentAsNumber();
    double distanceMax = options.at("distance").getArgumentAsNumber();



    SketchFingerPrint::Parameters parameters;

    if ( sketchParameterFingerPrintSetup(parameters, *(Command *)this) )
    {
    	return 1;
    }

    SketchFingerPrint sketchFingerPrintRef;
    
    uint64_t lengthMax;
    double randomChance;
    int kMin;
    string lengthMaxName;
    int warningCount = 0;
    
    const string & fileReference = arguments[0];
    
    bool isSketch = hasSuffixFingerPrint(fileReference, suffixSketch);
    
    if ( isSketch )
    {
        if ( options.at("kmer").active )
        {
            cerr << "ERROR: The option -" << options.at("kmer").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        
        if ( options.at("noncanonical").active )
        {
            cerr << "ERROR: The option -" << options.at("noncanonical").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        
        if ( options.at("protein").active )
        {
            cerr << "ERROR: The option -" << options.at("protein").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
        
        if ( options.at("alphabet").active )
        {
            cerr << "ERROR: The option -" << options.at("alphabet").identifier << " cannot be used when a sketch is provided; it is inherited from the sketch." << endl;
            return 1;
        }
    }
    else
    {
        cerr << "Sketching " << fileReference << " (provide sketch file made with \"mash sketch\" to skip)...";
    }
    
    vector<string> refArgVector;
    refArgVector.push_back(fileReference);

     
    bool tagMSH = containsMSH(refArgVector);
    bool tagTXT = containsTXT(refArgVector);

    if(tagMSH){

        sketchFingerPrintRef.initFromFingerPrintFiles(refArgVector, parameters); // Nuova funzione per fingerprint

    }
    else if (tagTXT)
    {
        sketchFingerPrintRef.initFromFingerprints(refArgVector, parameters); // Nuova funzione per fingerprint
    }
    
    double lengthThreshold = (parameters.warning * sketchFingerPrintRef.getKmerSpace()) / (1. - parameters.warning);
    
    if ( isSketch )
    {
        if ( options.at("sketchSize").active )
        {
            if ( parameters.reads && parameters.minHashesPerWindow != sketchFingerPrintRef.getMinHashesPerWindow() )
            {
                cerr << "ERROR: The sketch size must match the reference when using a bloom filter (leave this option out to inherit from the reference sketch)." << endl;
                return 1;
            }
        }
        
        parameters.minHashesPerWindow = sketchFingerPrintRef.getMinHashesPerWindow();
        parameters.kmerSize = sketchFingerPrintRef.getKmerSize();
        parameters.noncanonical = sketchFingerPrintRef.getNoncanonical();
        parameters.preserveCase = sketchFingerPrintRef.getPreserveCase();
        parameters.seed = sketchFingerPrintRef.getHashSeed();
        
        string alphabet;
        sketchFingerPrintRef.getAlphabetAsString(alphabet);
        setAlphabetFingerPrintFromString(parameters, alphabet.c_str());
    }
    else
    {
        for ( uint64_t i = 0; i < sketchFingerPrintRef.getReferenceCount(); i++ )
        {
            uint64_t length = sketchFingerPrintRef.getReference(i).subSketch_list.size();
        
            if ( length > lengthThreshold )
            {
                if ( warningCount == 0 || length > lengthMax )
                {
                    lengthMax = length;
                    lengthMaxName = sketchFingerPrintRef.getReference(i).name;
                    randomChance = sketchFingerPrintRef.getRandomKmerChance(i);
                    kMin = sketchFingerPrintRef.getMinKmerSize(i);
                }
            
                warningCount++;
            }
        }
    
        cerr << "done.\n";
    }
    
    if ( table )
    {
        cout << "#query";
        
        for ( int i = 0; i < sketchFingerPrintRef.getReferenceCount(); i++ )
        {
            cout << '\t' << sketchFingerPrintRef.getReference(i).name;
        }
        
        cout << endl;
    }
    
    bool method_exact = options.at("exact-similarity").active;
    if(method_exact){
        cout<< "Sono qua Metodo 1!"<< endl;
    
    ThreadPool<CompareFingerPrintInput, CompareFingerPrintOutput> threadPool(compareFingerPrintExcatSimilarity, threads);
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
    
    SketchFingerPrint sketchFingerPrintQuery;
    

    // Applico lo stesso ragionamento fatto in precedenza 
    if(tagMSH){

        sketchFingerPrintQuery.initFromFingerPrintFiles(queryFiles, parameters); // Nuova funzione per fingerprint
    }
    else if (tagTXT)
    {
        sketchFingerPrintQuery.initFromFingerprints(queryFiles, parameters); // Nuova funzione per fingerprint
    }
    
    uint64_t pairCount = sketchFingerPrintRef.getReferenceCount() * sketchFingerPrintQuery.getReferenceCount();

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
    
    uint64_t iFloor = pairsPerThread / sketchFingerPrintRef.getReferenceCount();
    uint64_t iMod = pairsPerThread % sketchFingerPrintRef.getReferenceCount();
    
    for ( uint64_t i = 0, j = 0; i < sketchFingerPrintQuery.getReferenceCount(); i += iFloor, j += iMod )
    {
        if ( j >= sketchFingerPrintRef.getReferenceCount() )
        {
            if ( i == sketchFingerPrintQuery.getReferenceCount() - 1 )
            {
                break;
            }
            
            i++;
            j -= sketchFingerPrintRef.getReferenceCount();
        }
        
        threadPool.runWhenThreadAvailable(new CompareFingerPrintInput(sketchFingerPrintRef, sketchFingerPrintQuery, j, i, pairsPerThread, parameters, distanceMax, pValueMax));
        
        while ( threadPool.outputAvailable() )
        {
            writeFingerPrintOutput(threadPool.popOutputWhenAvailable(), table, comment);
        }
    }
    
    while ( threadPool.running() )
    {
        writeFingerPrintOutput(threadPool.popOutputWhenAvailable(), table, comment);
    }
    
    if ( warningCount > 0 && ! parameters.reads )
    {
    	warnKmerFingerPrintSize(parameters, *this, lengthMax, lengthMaxName, randomChance, kMin, warningCount);
    }

    }else{


        ThreadPool<CompareFingerPrintInput, CompareFingerPrintOutput> threadPool(compareFingerPrintWithPercentageSimilarity, threads);
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
    
    SketchFingerPrint sketchFingerPrintQuery;
    

    // Applico lo stesso ragionamento fatto in precedenza 
    if(tagMSH){

        sketchFingerPrintQuery.initFromFingerPrintFiles(queryFiles, parameters); // Nuova funzione per fingerprint
    }
    else if (tagTXT)
    {
        sketchFingerPrintQuery.initFromFingerprints(queryFiles, parameters); // Nuova funzione per fingerprint
    }
    
    uint64_t pairCount = sketchFingerPrintRef.getReferenceCount() * sketchFingerPrintQuery.getReferenceCount();

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
    
    uint64_t iFloor = pairsPerThread / sketchFingerPrintRef.getReferenceCount();
    uint64_t iMod = pairsPerThread % sketchFingerPrintRef.getReferenceCount();
    
    for ( uint64_t i = 0, j = 0; i < sketchFingerPrintQuery.getReferenceCount(); i += iFloor, j += iMod )
    {
        if ( j >= sketchFingerPrintRef.getReferenceCount() )
        {
            if ( i == sketchFingerPrintQuery.getReferenceCount() - 1 )
            {
                break;
            }
            
            i++;
            j -= sketchFingerPrintRef.getReferenceCount();
        }
        
        threadPool.runWhenThreadAvailable(new CompareFingerPrintInput(sketchFingerPrintRef, sketchFingerPrintQuery, j, i, pairsPerThread, parameters, distanceMax, pValueMax));
        
        while ( threadPool.outputAvailable() )
        {
            writeFingerPrintOutput(threadPool.popOutputWhenAvailable(), table, comment);
        }
    }
    
    while ( threadPool.running() )
    {
        writeFingerPrintOutput(threadPool.popOutputWhenAvailable(), table, comment);
    }
    
    if ( warningCount > 0 && ! parameters.reads )
    {
    	warnKmerFingerPrintSize(parameters, *this, lengthMax, lengthMaxName, randomChance, kMin, warningCount);
    }

    }
    
    
    return 0; 
}



void CommandDistance::writeOutput(CompareOutput * output, bool table, bool comment) const
{
    uint64_t i = output->indexQuery;
    uint64_t j = output->indexRef;
    
    for ( uint64_t k = 0; k < output->pairCount && i < output->sketchQuery.getReferenceCount(); k++ )
    {
        const CompareOutput::PairOutput * pair = &output->pairs[k];
        
        if ( table && j == 0 )
        {
            cout << output->sketchQuery.getReference(i).name;
        }
        
        if ( table )
        {
            cout << '\t';
    
            if ( pair->pass )
            {
                cout << pair->distance;
            }
        }
        else if ( pair->pass )
        {
            cout << output->sketchRef.getReference(j).name;
            
            if ( comment )
            {
                cout << ':' << output->sketchRef.getReference(j).comment;
            }
            
            cout << '\t' << output->sketchQuery.getReference(i).name;
            
            if ( comment )
            {
                cout << ':' << output->sketchQuery.getReference(i).comment;
            }
            
            cout << '\t' << pair->distance << '\t' << pair->pValue << '\t' << pair->numer << '/' << pair->denom << endl;
        }
    
        j++;
        
        if ( j == output->sketchRef.getReferenceCount() )
        {
            if ( table )
            {
                cout << endl;
            }
            
            j = 0;
            i++;
        }
    }
    
    delete output;
}

CommandDistance::CompareOutput * compare(CommandDistance::CompareInput * input)
{
    const Sketch & sketchRef = input->sketchRef;
    const Sketch & sketchQuery = input->sketchQuery;
    
    CommandDistance::CompareOutput * output = new CommandDistance::CompareOutput(input->sketchRef, input->sketchQuery, input->indexRef, input->indexQuery, input->pairCount);
    
    uint64_t sketchSize = sketchQuery.getMinHashesPerWindow() < sketchRef.getMinHashesPerWindow() ?
        sketchQuery.getMinHashesPerWindow() :
        sketchRef.getMinHashesPerWindow();
    
    uint64_t i = input->indexQuery;
    uint64_t j = input->indexRef;
    
    for (uint64_t k = 0; k < input->pairCount && i < sketchQuery.getReferenceCount(); k++) {
        try {
            compareSketches(&output->pairs[k], sketchRef.getReference(j), sketchQuery.getReference(i), sketchSize, sketchRef.getKmerSize(), sketchRef.getKmerSpace(), input->maxDistance, input->maxPValue);
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: out_of_range exception caught: " << e.what() << std::endl;
        }
        j++;
        if (j == sketchRef.getReferenceCount()) {
            j = 0;
            i++;
        }
    }
    
    return output;
    
}

void compareSketches(CommandDistance::CompareOutput::PairOutput * output, const Sketch::Reference & refRef, const Sketch::Reference & refQry, uint64_t sketchSize, int kmerSize, double kmerSpace, double maxDistance, double maxPValue)
{
    uint64_t i = 0;
    uint64_t j = 0;
    uint64_t common = 0;
    uint64_t denom = 0;
    const HashList & hashesSortedRef = refRef.hashesSorted;
    const HashList & hashesSortedQry = refQry.hashesSorted;
    
    output->pass = false;
    
    while (denom < sketchSize && i < hashesSortedRef.size() && j < hashesSortedQry.size()) {
        if (hashLessThan(hashesSortedRef.at(i), hashesSortedQry.at(j), hashesSortedRef.get64())) {
            i++;
        } else if (hashLessThan(hashesSortedQry.at(j), hashesSortedRef.at(i), hashesSortedRef.get64())) {
            j++;
        } else {
            i++;
            j++;
            common++;
        }
        denom++;
    }
    
    if (denom < sketchSize) {
        // complete the union operation if possible
        if (i < hashesSortedRef.size()) {
            denom += hashesSortedRef.size() - i;
        }
        if (j < hashesSortedQry.size()) {
            denom += hashesSortedQry.size() - j;
        }
        if (denom > sketchSize) {
            denom = sketchSize;
        }
    }
    
    double distance;
    double jaccard = double(common) / denom;
    cout<< "JACCARD : "<< jaccard <<endl; 
    cout<< "K-mer Size : "<< kmerSize <<endl;
    
    if (common == denom) { // avoid -0
        distance = 0;
    } else if (common == 0) { // avoid inf
        distance = 1.;
    } else {
        distance = -log(2 * jaccard / (1. + jaccard)) / kmerSize;
        if (distance > 1) {
            distance = 1;
        }
    }
    
    if (maxDistance >= 0 && distance > maxDistance) {
        return;
    }
    
    output->numer = common;
    output->denom = denom;
    output->distance = distance;
    output->pValue = pValue(common, refRef.length, refQry.length, kmerSpace, denom);


    cout << "Distance : "<< (double) output->distance <<endl;
    cout<<"P-Value : "<<(double) output->pValue <<endl;

    cout<< "\n ------------------ \n"<<endl;
    
    if (maxPValue >= 0 && output->pValue > maxPValue) {
        return;
    }
    
    output->pass = true;

}



// ------------------------------------ FingerPrint-Jaccard Exact Similarity  ----------------------------------------------//

void CommandDistance::writeFingerPrintOutput(CompareFingerPrintOutput * output, bool table, bool comment) const
{
    uint64_t i = output->indexQuery;
    uint64_t j = output->indexRef;
    
    for (uint64_t k = 0; k < output->pairCount && i < output->sketchQuery.getReferenceCount(); k++)
    {
        const CompareFingerPrintOutput::PairOutput * pair = &output->pairs[k];
        
        if (table && j == 0)
        {
            cout << output->sketchQuery.getReference(i).name;
        }
        
        if (table)
        {
            cout << '\t';
    
            if (pair->pass)
            {
                cout << (pair->distance * 100) << '%';  // Visualizzazione della distance in percentuale
            }
        }
        else if (pair->pass)
        {
            cout << output->sketchRef.getReference(j).name;
            
            if (comment)
            {
                cout << ':' << output->sketchRef.getReference(j).comment;
            }
            
            cout << '\t' << output->sketchQuery.getReference(i).name;
            
            if (comment)
            {
                cout << ':' << output->sketchQuery.getReference(i).comment;
            }
            
            cout << '\t' 
                 << (pair->distance * 100) << '%' << '\t' // Visualizzazione della distance in percentuale
                 << (pair->pValue * 100) << '%' << '\t' // Visualizzazione del pValue in percentuale
                 << pair->numer << '/' << pair->denom << endl;
        }
    
        j++;
        
        if (j == output->sketchRef.getReferenceCount())
        {
            if (table)
            {
                cout << endl;
            }
            
            j = 0;
            i++;
        }
    }
    
    delete output;
}


CommandDistance::CompareFingerPrintOutput* compareFingerPrintExcatSimilarity(CommandDistance::CompareFingerPrintInput* input) {
    const SketchFingerPrint& sketchRef = input->sketchRef;
    const SketchFingerPrint& sketchQuery = input->sketchQuery;

    CommandDistance::CompareFingerPrintOutput* output = new CommandDistance::CompareFingerPrintOutput(input->sketchRef, input->sketchQuery, input->indexRef, input->indexQuery, input->pairCount);

    uint64_t sketchSize = std::min(sketchQuery.getMinHashesPerWindow(), sketchRef.getMinHashesPerWindow());

    uint64_t i = input->indexQuery;
    uint64_t j = input->indexRef;

    for (uint64_t k = 0; k < input->pairCount && i < sketchQuery.getReferenceCount(); k++) {
        try {
            const auto& refRef = sketchRef.getReference(j);
            const auto& refQry = sketchQuery.getReference(i);

            if (refRef.subSketch_list.size() != refQry.subSketch_list.size()) {
                output->pairs[k].pass = false;
                continue;
            }

            uint64_t totalCommon = 0;
            uint64_t totalDenom = 0;

            for (size_t idx = 0; idx < refRef.subSketch_list.size(); idx++) {
                const HashList& hashesSortedRef = refRef.subSketch_list.at(idx);
                const HashList& hashesSortedQry = refQry.subSketch_list.at(idx);

                uint64_t common = 0;
                uint64_t denom = 0;

                compareFingerPrintSketches(&output->pairs[k], hashesSortedRef, hashesSortedQry, sketchSize, sketchRef.getKmerSize(), sketchRef.getKmerSpace(), input->maxDistance, input->maxPValue, common, denom);

                totalCommon += common;
                totalDenom += denom;
            }

            output->pairs[k].numer = totalCommon;
            output->pairs[k].denom = totalDenom;

            double distance;
            double jaccard = double(totalCommon) / totalDenom;

            cout<< "JACCARD : "<< jaccard <<endl; 
            cout<< "K-mer Size : "<< sketchRef.getKmerSize() <<endl;

            if (totalCommon == totalDenom) {
                distance = 0;
            } else if (totalCommon == 0) {
                distance = 1.;
            } else {
                distance = -log(2 * jaccard / (1. + jaccard)) / sketchRef.getKmerSize();
                if (distance > 1) {
                    distance = 1;
                }
            }

            if (input->maxDistance >= 0 && distance > input->maxDistance) {
                output->pairs[k].pass = false;
                continue;
            }

            output->pairs[k].distance = distance;
            output->pairs[k].pValue = pValue(totalCommon, refRef.subSketch_list.size(), refQry.subSketch_list.size(), sketchRef.getKmerSpace(), totalDenom);

            cout << "Distance : "<< (double) distance<<endl;
            cout<<"P-Value : "<<(double) output->pairs[k].pValue <<endl;

            cout<< "\n ------------------ \n"<<endl;


            if (input->maxPValue >= 0 && output->pairs[k].pValue > input->maxPValue) {
                output->pairs[k].pass = false;
                continue;
            }

            output->pairs[k].pass = true;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: out_of_range exception caught: " << e.what() << std::endl;
        }
        j++;
        if (j == sketchRef.getReferenceCount()) {
            j = 0;
            i++;
        }
    }
    return output;
}


void compareFingerPrintSketches(CommandDistance::CompareFingerPrintOutput::PairOutput* output, const HashList& hashesSortedRef, const HashList& hashesSortedQry, uint64_t sketchSize, int kmerSize, double kmerSpace, double maxDistance, double maxPValue, uint64_t& common, uint64_t& denom) {
    uint64_t i = 0;
    uint64_t j = 0;
    common = 0;
    denom = 0;

    while (denom < sketchSize && i < hashesSortedRef.size() && j < hashesSortedQry.size()) {
        if (hashLessThan(hashesSortedRef.at(i), hashesSortedQry.at(j), hashesSortedRef.get64())) {
            i++;
        } else if (hashLessThan(hashesSortedQry.at(j), hashesSortedRef.at(i), hashesSortedRef.get64())) {
            j++;
        } else {
            i++;
            j++;
            common++;
        }
        denom++;
    }

    if (denom < sketchSize) {
        if (i < hashesSortedRef.size()) {
            denom += hashesSortedRef.size() - i;
        }
        if (j < hashesSortedQry.size()) {
            denom += hashesSortedQry.size() - j;
        }
        if (denom > sketchSize) {
            denom = sketchSize;
        }
    }
}


//--------------------------------------- FingerPrint - Jaccard With % of Similarity -------------------------------------------------------// 


CommandDistance::CompareFingerPrintOutput* compareFingerPrintWithPercentageSimilarity(CommandDistance::CompareFingerPrintInput* input) {
    const SketchFingerPrint& sketchRef = input->sketchRef;
    const SketchFingerPrint& sketchQuery = input->sketchQuery;

    CommandDistance::CompareFingerPrintOutput* output = new CommandDistance::CompareFingerPrintOutput(input->sketchRef, input->sketchQuery, input->indexRef, input->indexQuery, input->pairCount);

    uint64_t i = input->indexQuery;
    uint64_t j = input->indexRef;

    for (uint64_t k = 0; k < input->pairCount && i < sketchQuery.getReferenceCount(); k++) {
        try {


            const auto& refRef = sketchRef.getReference(j);
            const auto& refQry = sketchQuery.getReference(i);

            uint64_t totalCommon = 0;
            uint64_t totalDenom = 0;

            double jaccardIndex = jaccardSimilarityAndCommon(refRef.subSketch_list, refQry.subSketch_list, totalCommon, totalDenom);

            output->pairs[k].numer = totalCommon;
            output->pairs[k].denom = totalDenom;
            output->pairs[k].distance = 1.0 - jaccardIndex;
            output->pairs[k].pValue = pValue(totalCommon, refRef.subSketch_list.size(), refQry.subSketch_list.size(), sketchRef.getKmerSpace(), totalDenom);

            if (input->maxDistance >= 0 && output->pairs[k].distance > input->maxDistance) {
                output->pairs[k].pass = false;
                continue;
            }

            if (input->maxPValue >= 0 && output->pairs[k].pValue > input->maxPValue) {
                output->pairs[k].pass = false;
                continue;
            }

            output->pairs[k].pass = true;

        } catch (const std::out_of_range& e) {
            std::cerr << "Error: out_of_range exception caught: " << e.what() << std::endl;
        }
        j++;
        if (j == sketchRef.getReferenceCount()) {
            j = 0;
            i++;
        }
    }
    return output;
}



double jaccardSimilarityAndCommon(

    const std::vector<HashList>& set1,

    const std::vector<HashList>& set2,
    
    uint64_t& totalCommon,
    
    uint64_t& totalDenom) {
    
    // Identifica quale set è più grande
    vector<HashList> largerSet = set1;
    vector<HashList> smallerSet= set2;

    if(set1.size()>set2.size()){

        largerSet = set1;
        smallerSet = set2;

        cout<<"Sono qui Maggiore!"<<endl;

    }else if(set1.size()<set2.size()){
        
        largerSet = set2;
        smallerSet= set1;

        cout<<"Sono qui Minore !"<<endl;

    }else{

        cout<<"Sono qui Uguale !"<<endl;
        largerSet = set2;
        smallerSet = set1;
    }



    // Usa un insieme per tenere traccia degli elementi già trovati nell'intersezione
    /*std::unordered_set<size_t> foundIndices; // Indici di HashList già trovati nel largerSet

    // Calcola l'intersezione e l'unione
    for (const auto& list1 : largerSet) {
        
        bool foundSimilar = false;

        // Controlla se list1 è simile a uno degli elementi in smallerSet
        for (const auto& list2 : smallerSet) {
            if (areHashListsSimilar(list1, list2)) {
                intersectionSize++; // Incrementa la dimensione dell'intersezione
                foundSimilar = true;
                totalCommon++; // Aggiorna totalCommon
                foundIndices.insert(&list1 - &largerSet[0]); // Aggiungi l'indice di list1
                break; // Un match trovato, esci dal ciclo interno
            }
        }
        unionSize++; // Aggiungi a unionSize per ogni elemento in largerSet
        totalDenom++; // Aggiornamento di totalDenom
    }

    // Aggiungi gli elementi del smallerSet che non sono stati trovati nel largerSet
    for (const auto& list2 : smallerSet) {
        // Verifica se list2 è simile a un elemento già trovato in largerSet
        bool foundSimilar = false;
    
        for (const auto& list1 : largerSet) {
            if (areHashListsSimilar(list1, list2)) {
                
                foundSimilar = true;
                //totalDenom++; // Aggiornamento di totalDenom quando viene trovata una corrispondenza
                break;
            }
        }
        if (!foundSimilar) {
            unionSize++; // Aggiungi a unionSize se list2 non è stato trovato
            totalDenom++;
        }
    }

    // Verifica di evitare divisione per zero
    if (unionSize == 0) {
        return 0.0; // Nessuna similarità se l'unione è zero
    }
    
    //totalDenom = totalDenom - totalCommon;
    
    cout<<"Total Denom : "<<totalDenom<<endl;
    cout<<"Total Common :"<<totalCommon<<endl;

    cout<<"Total Size Intersection :"<<intersectionSize<<endl;
    cout<<"Total Size Union :"<<unionSize << endl;

    // Calcola e restituisce il coefficiente di similarità di Jaccard
    return static_cast<double>(intersectionSize) / unionSize;*/

    cout<< "################################################# INTERSEZIONE ############################################" <<endl;
    // Calcolo dell'Intersezione 
    int intersectionSize = calculateIntersection(largerSet, smallerSet);

    cout<< "################################################# UNIONE ############################################" <<endl;

    // Calcolo dell'Unione 
    int unionSize = calculateUnion(largerSet, smallerSet);

    // Verifica di evitare divisione per zero
    if (unionSize == 0) {
        return 0.0; // Nessuna similarità se l'unione è zero
    }

    std::cout << "Total Size Intersection: " << intersectionSize << std::endl;
    std::cout << "Total Size Union: " << unionSize << std::endl;

    totalCommon = intersectionSize;
    totalDenom = unionSize;

    // Calcola e restituisce il coefficiente di similarità di Jaccard
    return static_cast<double>(intersectionSize) / unionSize;
}



// Funzione per calcolare l'unione
int calculateUnion(const std::vector<HashList>& largerSet, const std::vector<HashList>& smallerSet) {

    int unionSize = 0;
    
    std::unordered_set<size_t> foundIndices;

    // Analizza le liste di largerSet
    for (const auto& list1 : largerSet) {
        
        std::cout << "Analizzando list1 (largerSet): " << list1.toString() << std::endl;
        
        unionSize++;
        for (const auto& list2 : smallerSet) {
            
            std::cout << "  Contro list2 (smallerSet): " << list2.toString() << std::endl;

            if (areHashListsSimilar(list1, list2)) {
                foundIndices.insert(&list1 - &largerSet[0]);
                std::cout << "    Trovata similarità di unione list1 e list2" << std::endl;
                break;
            }
        }
    }

    // Analizza le liste di smallerSet
    for (const auto& list2 : smallerSet) {
        std::cout << "Analizzando list2 (smallerSet): " << list2.toString() << std::endl;
        bool foundSimilar = false;
        for (const auto& list1 : largerSet) {
            std::cout << "  Contro list1 (largerSet): " << list1.toString() << std::endl;
            if (areHashListsSimilar(list1, list2)) {
                
                foundSimilar = true;
                
                std::cout << "    Trovata similarità di unione list2 e list1" << std::endl;
                
                break;
            }
        }
        if (!foundSimilar) {

            unionSize++;
            
            std::cout << "    list2 non trovata in largerSet, aumentando l'unione." << std::endl;
        }
    }

    std::cout << "Size dell'Unione: " << unionSize << std::endl;
    
    return unionSize;
}



// Funzione per calcolare l'intersezione
int calculateIntersection(const std::vector<HashList>& largerSet, const std::vector<HashList>& smallerSet) {

    int intersectionSize = 0;
    
    std::unordered_set<size_t> foundIndices; // Indici di HashList già trovati nel largerSet
    std::unordered_set<size_t> checkedIndices; // Indici di HashList già verificati in smallerSet

    for (const auto& list1 : largerSet) {
        std::cout << "Analizzando list1: " << list1.toString() << std::endl;
        for (const auto& list2 : smallerSet) {
            size_t list2Index = &list2 - &smallerSet[0]; // Calcola l'indice di list2
            // Se abbiamo già verificato questo list2, salta al prossimo
            if (checkedIndices.find(list2Index) != checkedIndices.end()) {
                std::cout << "    list2 già verificato: " << list2.toString() << std::endl;
                continue;
            }

            std::cout << "  Contro list2: " << list2.toString() << std::endl;
            if (areHashListsSimilar(list1, list2)) {
                std::cout << "    Trovata similarità di intersezione tra list1 e list2" << std::endl;
                intersectionSize++;
                foundIndices.insert(&list1 - &largerSet[0]); // Aggiungi l'indice di list1
                checkedIndices.insert(list2Index); // Aggiungi list2 all'insieme degli indici controllati
                break; // Un match trovato, esci dal ciclo interno
            }
        }
    }
    std::cout << "Size dell'Intersezione: " << intersectionSize << std::endl;
    return intersectionSize;
}


// ########### CALCOLO DELLA DISTANZA DI HAMMING TRA I VARI VETTORI DI HASH ############


// Funzioni di confronto per hash a 64 bit e 32 bit
int hashEquals64(uint64_t hash1, uint64_t hash2, int hashSize) {
    return (hash1 == hash2) ? 0 : hashSize;
}

int hashEquals32(uint32_t hash1, uint32_t hash2, int hashSize) {
    return (hash1 == hash2) ? 0 : hashSize;
}

/* calcolo dove avviene la funzione di calcolo di Hamming Distance
/*
int distanceBetweenHashLists(const HashList& list1, const HashList& list2) {
    // Calcola la distanza totale tra due HashList
    int totalDistance = 0;
    size_t minSize = std::min(list1.size(), list2.size());

    // Determina se entrambe le liste utilizzano hash a 64 bit
    bool tagUse64 = list1.get64() && list2.get64();
    if (list1.get64() != list2.get64()) {
        throw std::invalid_argument("Both HashLists must use the same hash size (either 64-bit or 32-bit).");
    }
    int hashSize = tagUse64 ? 64 : 32; // Dimensione dell'hash

    // Calcola la distanza per i primi minSize elementi
    for (size_t i = 0; i < minSize; ++i) {
        totalDistance += tagUse64 ? hashEquals64(list1.at(i).hash64, list2.at(i).hash64, hashSize)
                                  : hashEquals32(list1.at(i).hash32, list2.at(i).hash32, hashSize);
    }

    // Calcola la distanza per gli elementi rimanenti
    size_t remainingElements = std::max(list1.size(), list2.size()) - minSize;
    totalDistance += remainingElements * hashSize;

    return totalDistance; // Restituisce la distanza totale

}
*/

// FUNZIONE PER IL CALCOLO DI JACCARD
/*
int distanceBetweenHashLists(const HashList& list1, const HashList& list2) {
    // Calcola la distanza di Jaccard tra due HashList
    int intersectionCount = 0;
    int unionCount = 0;

    // Verifica che entrambe le liste utilizzino lo stesso tipo di hash (64-bit o 32-bit)
    bool tagUse64 = list1.get64() && list2.get64();
    if (list1.get64() != list2.get64()) {
        throw std::invalid_argument("Both HashLists must use the same hash size (either 64-bit or 32-bit).");
    }

    // Utilizza un set per contare l'unione e l'intersezione degli hash, distinguendo tra 32-bit e 64-bit
    if (tagUse64) {
        // Se usiamo hash a 64-bit
        std::unordered_set<uint64_t> unionSet64;
        std::unordered_set<uint64_t> intersectionSet64;

        // Aggiunge gli hash dalla prima lista all'unionSet
        for (size_t i = 0; i < list1.size(); ++i) {
            unionSet64.insert(list1.at(i).hash64);
        }

        // Controlla l'intersezione e aggiunge gli hash dalla seconda lista all'unionSet
        for (size_t i = 0; i < list2.size(); ++i) {
            uint64_t hash = list2.at(i).hash64;
            if (unionSet64.find(hash) != unionSet64.end()) {
                intersectionSet64.insert(hash);
            }
            unionSet64.insert(hash);
        }

        // Calcola l'intersezione e l'unione per hash a 64-bit
        intersectionCount = intersectionSet64.size();
        unionCount = unionSet64.size();
    } else {
        // Se usiamo hash a 32-bit
        std::unordered_set<uint32_t> unionSet32;
        std::unordered_set<uint32_t> intersectionSet32;

        // Aggiunge gli hash dalla prima lista all'unionSet
        for (size_t i = 0; i < list1.size(); ++i) {
            unionSet32.insert(list1.at(i).hash32);
        }

        // Controlla l'intersezione e aggiunge gli hash dalla seconda lista all'unionSet
        for (size_t i = 0; i < list2.size(); ++i) {
            uint32_t hash = list2.at(i).hash32;
            if (unionSet32.find(hash) != unionSet32.end()) {
                intersectionSet32.insert(hash);
            }
            unionSet32.insert(hash);
        }

        // Calcola l'intersezione e l'unione per hash a 32-bit
        intersectionCount = intersectionSet32.size();
        unionCount = unionSet32.size();
    }

    // Se non ci sono elementi nell'unione, la distanza è 0
    if (unionCount == 0) {
        return 0;
    }

    // Calcola e restituisce la distanza di Jaccard
    double jaccardDistance = 1.0 - (double(intersectionCount) / unionCount);
    return static_cast<int>(jaccardDistance);
}*/


//FUNZIONE DI DISTANZA TRA VETTORI CON DISTANZA DI EDIT
#include "HashList.h"
#include <vector>
#include <stdexcept>
#include <algorithm>

// Funzione per calcolare la distanza di Levenshtein tra due HashList
int distanceBetweenHashLists(const HashList& list1, const HashList& list2) {
    size_t m = list1.size();
    size_t n = list2.size();

    // Verifica se entrambe le liste utilizzano hash a 64 bit o 32 bit
    bool use64 = list1.get64() && list2.get64();
    if (list1.get64() != list2.get64()) {
        throw std::invalid_argument("Both HashLists must use the same hash size (either 64-bit or 32-bit).");
    }

    // Inizializzazione della matrice (m+1) x (n+1)
    std::vector<std::vector<int>> matrix(m + 1, std::vector<int>(n + 1, 0));

    // Inizializzazione della prima riga e della prima colonna
    for (size_t i = 0; i <= m; ++i) {
        matrix[i][0] = static_cast<int>(i);
    }
    for (size_t j = 0; j <= n; ++j) {
        matrix[0][j] = static_cast<int>(j);
    }

    // Popolazione della matrice
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            int cost;
            if (use64) {
                // Confronta hash a 64 bit
                cost = (list1.at(i - 1).hash64 == list2.at(j - 1).hash64) ? 0 : 1;
            } else {
                // Confronta hash a 32 bit
                cost = (list1.at(i - 1).hash32 == list2.at(j - 1).hash32) ? 0 : 1;
            }

            // Calcola il minimo tra eliminazione, inserimento e sostituzione
            matrix[i][j] = std::min({
                matrix[i - 1][j] + 1,      // Eliminazione
                matrix[i][j - 1] + 1,      // Inserimento
                matrix[i - 1][j - 1] + cost // Sostituzione
            });
        }
    }

    // Stampa della matrice
    std::cout << "Matrice della Distanza di Levenshtein:\n";
    std::cout << std::setw(8) << "";
    for (size_t j = 0; j <= n; ++j) {
        if (j > 0) {
            if (use64)
                std::cout << std::setw(10) << list2.at(j - 1).hash64;
            else
                std::cout << std::setw(10) << list2.at(j - 1).hash32;
        } else {
            std::cout << std::setw(10) << "-";
        }
    }
    std::cout << "\n";
    for (size_t i = 0; i <= m; ++i) {
        if (i > 0) {
            if (use64)
                std::cout << std::setw(8) << list1.at(i - 1).hash64;
            else
                std::cout << std::setw(8) << list1.at(i - 1).hash32;
        } else {
            std::cout << std::setw(8) << "-";
        }

        for (size_t j = 0; j <= n; ++j) {
            std::cout << std::setw(10) << matrix[i][j];
        }
        std::cout << "\n";
    }

    cout << "distanza minima: " << matrix[m][n] << endl;
    // La distanza finale si trova nell'ultima cella della matrice
    return matrix[m][n];
}




bool areHashListsSimilar(const HashList& list1, const HashList& list2) {



    int distance = distanceBetweenHashLists(list1, list2);
    

    bool tagUse64 = list1.get64() && list2.get64();
    //int totalBits = std::max(list1.size(), list2.size()) * (tagUse64 ? 64 : 32);
    int max_size = max(list1.size(), list2.size());
    int maxThreshold = max_size *THRESHOLD; // % del totale dei bit, parte bassa
    cout << "threshold da rispettare: " << maxThreshold << endl;

    return distance <= maxThreshold;
}
// ############# FINE CALCOLO DELLA DISTANZA DI HAMMING TRA I VARI VETTORI DI HASH ######################
//--------------------------------------- FingerPrint - Jaccard With % of Similarity -------------------------------------------------------// 



double pValue(uint64_t x, uint64_t lengthRef, uint64_t lengthQuery, double kmerSpace, uint64_t sketchSize)
{
    if ( x == 0 )
    {
        return 1.;
    }
    
    double pX = 1. / (1. + kmerSpace / lengthRef);
    double pY = 1. / (1. + kmerSpace / lengthQuery);
    
    double r = pX * pY / (pX + pY - pX * pY);
    
#ifdef USE_BOOST
    return cdf(complement(binomial(sketchSize, r), x - 1));
#else
    return gsl_cdf_binomial_Q(x - 1, r, sketchSize);
#endif

return 0; 
}


bool CommandDistance :: containsMSH(const std::vector<std::string>& strVec) const{
    
    bool flag = false;

    for (const auto& str : strVec) {
         flag = str.find(".msh") != std::string::npos;
    }

    return flag;

}


bool CommandDistance :: containsTXT(const std::vector<std::string>& strVec) const{
    bool flag = false;

    for (const auto& str : strVec) {
         flag = str.find(".txt") != std::string::npos;
    }

    return flag;
}



} // namespace mash
