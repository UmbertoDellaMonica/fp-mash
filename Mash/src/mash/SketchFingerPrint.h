#ifndef SketchFingerprint_h
#define SketchFingerprint_h

#include "mash/capnp/MinFingerPrintHash.capnp.h"
#include "robin_hood.h"
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include "MinHashHeap.h"
#include "ThreadPool.h"

static const char * capnpHeaderFingerPrint = "Cap'n Proto";
static const int capnpHeaderFingerPrintLength = strlen(capnpHeaderFingerPrint);

static const char * suffixFingerPrintSketch = ".msh";
static const char * suffixFingerPrintSketchWindowed = ".msw";

static const char * alphabetNucleotideFingerPrint = "ACGT";
static const char * alphabetProteinFingerPrint = "ACDEFGHIKLMNPQRSTVWY";

// FingerPrint section 
static const char * suffixFingerprint = ".txt";




class SketchFingerPrint
{
public:
    
    typedef uint64_t hash_t;
    
    struct Parameters
    {
        Parameters()
            :
            parallelism(1),
            kmerSize(0),
            alphabetSize(0),
            preserveCase(false),
            use64(false),
            seed(0),
            error(0),
            warning(0),
            minHashesPerWindow(0),
            windowSize(0),
            windowed(false),
            concatenated(false),
            noncanonical(false),
            reads(false),
            memoryBound(0),
            minCov(1),
            targetCov(0),
            genomeSize(0),
            counts(false)
        {
        	memset(alphabet, 0, 256);
        }
        
        Parameters(const Parameters & other)
            :
            parallelism(other.parallelism),
            kmerSize(other.kmerSize),
            alphabetSize(other.alphabetSize),
            preserveCase(other.preserveCase),
            use64(other.use64),
            seed(other.seed),
            error(other.error),
            warning(other.warning),
            minHashesPerWindow(other.minHashesPerWindow),
            windowSize(other.windowSize),
            windowed(other.windowed),
            concatenated(other.concatenated),
            noncanonical(other.noncanonical),
            reads(other.reads),
            memoryBound(other.memoryBound),
            minCov(other.minCov),
            targetCov(other.targetCov),
            genomeSize(other.genomeSize),
            counts(other.counts)
		{
			memcpy(alphabet, other.alphabet, 256);
		}
        
        int parallelism;
        int kmerSize;
        bool alphabet[256];
        uint32_t alphabetSize;
        bool preserveCase;
        bool use64;
        uint32_t seed;
        double error;
        double warning;
        uint64_t minHashesPerWindow;
        uint64_t windowSize;
        bool windowed;
        bool concatenated;
        bool noncanonical;
        bool reads;
        uint64_t memoryBound;
        uint32_t minCov;
        double targetCov;
        uint64_t genomeSize;
        bool counts;
        bool fingerprint = false;
    };
    
    
    
    struct Reference{
        // ID della reference 
        std::string id;
        // Nome reference 
        std::string name;
        // Commento della reference 
        std::string comment;
        // Lunghezza della reference 
        uint64_t length;
        // Lista di SubSketch
        std::vector<HashList> subSketch_list;
        bool use64;
        // Numero di subsketch
        std::vector<uint32_t> counts;
        // Valore bool dei vari hash in sorting 
        bool countsSorted;

    };
    

    
    struct SketchInput
    {

    	SketchInput(std::vector<std::string> fileNamesNew, char * seqNew, uint64_t lengthNew, const std::string & nameNew, const std::string & commentNew, const SketchFingerPrint::Parameters & parametersNew)
    	:
    	fileNames(fileNamesNew),
    	seq(seqNew),
    	length(lengthNew),
    	name(nameNew),
    	comment(commentNew),
    	parameters(parametersNew)
    	{}
    	
    	~SketchInput()
    	{
    		if ( seq != 0 )
    		{
	    		delete [] seq;
	    	}
    	}
    	
    	std::vector<std::string> fileNames;
    	
    	char * seq;
    	
    	uint64_t length;
    	
        std::string id;

    	std::string name;
    	
        std::string comment;
    	
    	SketchFingerPrint::Parameters parameters;
    };
    
    struct SketchOutput
    {
    	std::vector<Reference> references;    
    };
    

    /**
     * initFromFingerprintsWithSingleHash - Ogni riferimento ha un insieme di Hash 
     */
    void initFromFingerprintsWithSingleHash(const std::vector<std::string> & files, const Parameters & parametersNew);

    /**
     * initFromFingerprints - Ogni riferimento ha un insieme di vettori di HashList 
     */
    void initFromFingerprints(const std::vector<std::string> & files, const Parameters & parametersNew);
    void initFromFingerprintsSorted(const std::vector<std::string> & files, const Parameters & parametersNew);

    void getAlphabetAsString(std::string & alphabet) const; // Esiste 

    uint32_t getAlphabetSize() const {return parameters.alphabetSize;} 
    
    bool getConcatenated() const {return parameters.concatenated;}
    
    float getError() const {return parameters.error;}
    
    //int getHashCount() const {return lociByHash.size();}
    
    uint32_t getHashSeed() const {return parameters.seed;}
    
    float getMinHashesPerWindow() const {return parameters.minHashesPerWindow;}
	
    int getMinKmerSize(uint64_t reference) const;
	
    bool getPreserveCase() const {return parameters.preserveCase;}
	
    double getRandomKmerChance(uint64_t reference) const;
    
    
    
    const Reference & getReference(uint64_t index) const {return references.at(index);}
   

    uint64_t getReferenceCount() const {return references.size();}

    void getReferenceHistogram(uint64_t index, std::map<uint32_t, uint64_t> & histogram) const;
    
    
    uint64_t getReferenceIndex(std::string id) const;
    
    int getKmerSize() const {return parameters.kmerSize;}
    
    double getKmerSpace() const {return kmerSpace;}
    
    bool getUse64() const {return parameters.use64;}
    
    uint64_t getWindowSize() const {return parameters.windowSize;}
    
    bool getNoncanonical() const {return parameters.noncanonical;}
    
    bool hasHashCounts() const {return references.size() > 0 && references.at(0).counts.size() > 0;}
        
    //int initFromFiles(const std::vector<std::string> & files, const Parameters & parametersNew, int verbosity = 0, bool enforceParameters = false, bool contain = false);
    int initFromFingerPrintFiles(const std::vector<std::string> & files, const Parameters & parametersNew, int verbosity = 0, bool enforceParameters = false, bool contain = false);

    uint64_t initParametersFingerPrintsFromCapnp(const char * file);

    



    void setReferenceName(int i, const std::string name) {references[i].name = name;}
    
    void setReferenceComment(int i, const std::string comment) {references[i].comment = comment;}
	
    void setReferenceId(int i, const std::string id) { references[i].id = id; }
    
    bool sketchFileBySequence(FILE * file, ThreadPool<SketchFingerPrint::SketchInput, SketchFingerPrint::SketchOutput> * threadPool);
	
    void useThreadOutput(SketchOutput * output);
    
    void warnKmerSize(uint64_t lengthMax, const std::string & lengthMaxName, double randomChance, int kMin, int warningCount) const;

    int writeToCapnpFingerPrint(const char * file) const;

    void setSubSketchHashList(int i, int j, const HashList hashList) { references[i].subSketch_list.at(j) = hashList; };
    
private:
    
    // Crea l'indice delle FingerPrint() 
    void createIndexFingerPrint();
    
    // Vettore dei riferimenti dello sketch 
    std::vector<Reference> references;
    // Subsketch List all'interno dei programmi  
    std::vector<HashList> subSketchList;


    
    robin_hood::unordered_map<std::string, int> referenceIndecesById;

    std::unordered_map<std::string, std::pair<int, int>> subSketchVectorHashListById; // Indice per i subSketch
    
    Parameters parameters;

    double kmerSpace;

    std::string file;
};

void addMinHashes(MinHashHeap & minHashHeap, char * seq, uint64_t length, const SketchFingerPrint::Parameters & parameters);

//void getMinHashPositions(std::vector<SketchFingerPrint::PositionHash> & loci, char * seq, uint32_t length, const Sketch::Parameters & parameters, int verbosity = 0);

bool hasSuffixFingerPrint(std::string const & whole, std::string const & suffix);

SketchFingerPrint::SketchOutput * loadCapnpFingerPrint(SketchFingerPrint::SketchInput * input);

void reverseFingerPrintComplement(const char * src, char * dest, int length);

void setAlphabetFingerPrintFromString(SketchFingerPrint::Parameters & parameters, const char * characters);

void setMinHashesForReference(SketchFingerPrint::Reference & reference, const MinHashHeap & hashes);

SketchFingerPrint::SketchOutput * sketchFile(SketchFingerPrint::SketchInput * input);

SketchFingerPrint::SketchOutput * sketchSequence(SketchFingerPrint::SketchInput * input);


int defFingerPrint(int fdSource, int fdDest, int level);

int infFingerPrint(int fdSource, int fdDest);

void zerrFingerPrint(int ret);

#endif
