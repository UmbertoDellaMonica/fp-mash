#include "CommandDistance.h"
#include "Sketch.h"
#include "SketchFingerPrint.h"
#include "HashList.h"
#include <iostream>
#include <zlib.h>
#include "ThreadPool.h"
#include "sketchParameterSetup.h"
#include <math.h>

#ifdef USE_BOOST
    #include <boost/math/distributions/binomial.hpp>
    using namespace::boost::math;
#else
    #include <gsl/gsl_cdf.h>
#endif

using namespace::std;


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
            uint64_t length = sketchFingerPrintRef.getReference(i).length;
        
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


        cout<< "Sono qua - Metodo 2 !"<< endl;
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
    
    for ( uint64_t k = 0; k < output->pairCount && i < output->sketchQuery.getReferenceCount(); k++ )
    {
        const CompareFingerPrintOutput::PairOutput * pair = &output->pairs[k];
        
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
            
            cout << '\t' << (double) pair->distance << '\t' << (double) pair->pValue << '\t' << pair->numer << '/' << pair->denom << endl;
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


double jaccardSimilarityAndCommon(const std::vector<HashList>& set1, const std::vector<HashList>& set2, uint64_t& totalCommon, uint64_t& totalDenom) {
    int intersectionSize = 0;
    int unionSize = 0;

    for (const auto& list1 : set1) {
        bool foundSimilar = false;
        for (const auto& list2 : set2) {
            if (areHashListsSimilar(list1, list2)) {
                intersectionSize++;
                foundSimilar = true;
                totalCommon += 1; // Aggiornamento di totalCommon
                totalDenom += 1; // Aggiornamento di totalDenom
                break;
            }
        }
        unionSize++;
    }

    for (const auto& list2 : set2) {
        bool foundSimilar = false;
        for (const auto& list1 : set1) {
            if (areHashListsSimilar(list1, list2)) {
                foundSimilar = true;
                break;
            }
        }
        if (!foundSimilar) {
            unionSize++;
            totalDenom += 1; // Aggiornamento di totalDenom
        }
    }

    return static_cast<double>(intersectionSize) / unionSize;
}


int hashEquals64(uint64_t hash1, uint64_t hash2, uint64_t hash_size) {
    
    return (hash1 == hash2) ? 0 : hash_size;
}

int hashEquals32(uint32_t hash1, uint32_t hash2, uint32_t hash_size) {

    return (hash1 == hash2) ? 0 : hash_size;
}


int distanceBetweenHashLists(const HashList& list1, const HashList& list2)  {
    int totalDistance = 0;
    int minSize = std::min(list1.size(), list2.size());

    bool tagUse64 = list1.get64() && list2.get64() ? true: false;

    for (size_t i = 0; i < minSize; ++i) {
        
        if(tagUse64){
            totalDistance += hashEquals64(list1.at(i).hash64, list2.at(i).hash64,64);
        }else{
            totalDistance += hashEquals32(list1.at(i).hash32, list2.at(i).hash32,32);
        }
    }

    if(tagUse64){
        totalDistance += (list1.size() - minSize) * 64;
        totalDistance += (list2.size() - minSize) * 64;
    }else{
        totalDistance += (list1.size() - minSize) * 32;
        totalDistance += (list2.size() - minSize) * 32;
    }

    return totalDistance;
}

bool areHashListsSimilar(const HashList& list1, const HashList& list2)  {

    int distance = distanceBetweenHashLists(list1, list2);
    bool tagUse64 = list1.get64() && list2.get64() ? true: false;
    
    int totalBits = 0;

    if(tagUse64){
        int totalBits = std::max(list1.size(), list2.size()) * 64;
    }else{
        int totalBits = std::max(list1.size(), list2.size()) * 64;
    }
    return distance <= (totalBits * 0.25);
}


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
