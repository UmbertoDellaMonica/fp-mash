// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#ifndef Sketch_h
#define Sketch_h

#include "mash/capnp/MinHash.capnp.h"
#include "robin_hood.h"
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include "MinHashHeap.h"
#include "ThreadPool.h"

static const char * capnpHeader = "Cap'n Proto";
static const int capnpHeaderLength = strlen(capnpHeader);

static const char * suffixSketch = ".msh";
static const char * suffixSketchWindowed = ".msw";

static const char * alphabetNucleotide = "ACGT";
static const char * alphabetProtein = "ACDEFGHIKLMNPQRSTVWY";

// FingerPrint section 
static const char * suffixFingerprint = ".txt";




class Sketch
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
        bool fingerprint = false; // Nuovo parametro
    };
    
    struct PositionHash
    {
        PositionHash(uint32_t positionNew, hash_t hashNew) :
            position(positionNew),
            hash(hashNew)
            {}

        uint32_t position;
        hash_t hash;
    };
    
    struct Locus
    {
        Locus(uint32_t sequenceNew, uint32_t positionNew)
            :
            sequence(sequenceNew),
            position(positionNew)
            {}
        
        uint32_t sequence;
        uint32_t position;
    };
    


    /**
     * La struttura Reference è una parte essenziale della classe Sketch e rappresenta un singolo riferimento (o una singola sequenza) all'interno dello sketch. 
     * Questa struttura contiene vari parametri che descrivono dettagliatamente ogni riferimento:
     * 
     * Dettagli dei Parametri :
    name:

    Tipo: std::string
    Descrizione: Il nome del riferimento. Questo nome viene utilizzato per identificare univocamente la sequenza all'interno dello sketch. Potrebbe essere il nome di un gene, una proteina, o qualsiasi altro identificatore biologico.

    comment:

    Tipo: std::string
    Descrizione: Un commento opzionale associato al riferimento. Questo commento può contenere annotazioni descrittive aggiuntive sulla sequenza, come informazioni sull'origine della sequenza, note di ricerca, o altre annotazioni utili.

    length:

    Tipo: uint64_t
    Descrizione: La lunghezza del riferimento in termini di numero di basi (nucleotidi per sequenze di DNA/RNA) o residui (amminoacidi per sequenze proteiche). Questo parametro aiuta a determinare la dimensione della sequenza.

    hashesSorted:

    Tipo: HashList
    Descrizione: Una lista di hash che rappresentano i k-mer della sequenza. Gli hash sono ordinati. Ogni k-mer è una sotto-sequenza di lunghezza k della sequenza originale, e l'hashing è una tecnica per rappresentare queste sotto-sequenze in modo compatto.

    counts:

    Tipo: std::vector<uint32_t>
    Descrizione: Un vettore che contiene il conteggio dei k-mer nella sequenza. Ogni elemento di questo vettore rappresenta il numero di volte che un determinato k-mer appare nella sequenza. Questo parametro è utile per analisi quantitative e per calcolare frequenze relative.

    countsSorted:

    Tipo: bool
    Descrizione: Un flag booleano che indica se il vettore counts è ordinato. Se countsSorted è true, significa che i conteggi dei k-mer nel vettore counts sono ordinati. Questo può essere utile per alcune operazioni di ricerca e analisi che beneficiano dell'ordinamento.
    Uso della Struttura Reference
    La struttura Reference viene utilizzata per gestire e manipolare le informazioni associate a ciascun riferimento (sequenza) all'interno dello sketch.
    È parte integrante delle funzioni che calcolano, analizzano e visualizzano i dati di sketching, come l'inizializzazione degli sketch, il calcolo degli istogrammi, e altre analisi basate sui k-mer. */
    struct Reference
    {
        std::string id;
        std::string name;
        std::string comment;
        uint64_t length;
        HashList hashesSorted;
        std::vector<uint32_t> counts;
        bool countsSorted;
    };
    
    struct SketchInput
    {
    	SketchInput(std::vector<std::string> fileNamesNew, char * seqNew, uint64_t lengthNew, const std::string & nameNew, const std::string & commentNew, const Sketch::Parameters & parametersNew)
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
    	
    	std::string name;
    	std::string comment;
    	
    	Sketch::Parameters parameters;
    };
    
    struct SketchOutput
    {
    	std::vector<Reference> references;
	    std::vector<std::vector<PositionHash>> positionHashesByReference;
    };
    
    void initFromFingerprints(const std::vector<std::string> & files, const Parameters & parametersNew);
    void getAlphabetAsString(std::string & alphabet) const;
    uint32_t getAlphabetSize() const {return parameters.alphabetSize;}
    bool getConcatenated() const {return parameters.concatenated;}
    float getError() const {return parameters.error;}
    int getHashCount() const {return lociByHash.size();}
    uint32_t getHashSeed() const {return parameters.seed;}
    const std::vector<Locus> & getLociByHash(hash_t hash) const;
    float getMinHashesPerWindow() const {return parameters.minHashesPerWindow;}
	int getMinKmerSize(uint64_t reference) const;
	bool getPreserveCase() const {return parameters.preserveCase;}
	double getRandomKmerChance(uint64_t reference) const;
    const Reference & getReference(uint64_t index) const {return references.at(index);}
    /**
     * Ritorna il numero di reference che sono state trovate nel momento in cui lo sketch è stato inizializzato e letto
     */
    uint64_t getReferenceCount() const {return references.size();}

    void getReferenceHistogram(uint64_t index, std::map<uint32_t, uint64_t> & histogram) const;
    uint64_t getReferenceIndex(std::string id) const;
    int getKmerSize() const {return parameters.kmerSize;}
    double getKmerSpace() const {return kmerSpace;}
    bool getUse64() const {return parameters.use64;}
    uint64_t getWindowSize() const {return parameters.windowSize;}
    bool getNoncanonical() const {return parameters.noncanonical;}
    bool hasHashCounts() const {return references.size() > 0 && references.at(0).counts.size() > 0;}
    bool hasLociByHash(hash_t hash) const {return lociByHash.count(hash);}
    int initFromFiles(const std::vector<std::string> & files, const Parameters & parametersNew, int verbosity = 0, bool enforceParameters = false, bool contain = false);
    void initFromReads(const std::vector<std::string> & files, const Parameters & parametersNew);
    uint64_t initParametersFromCapnp(const char * file);
    void setReferenceName(int i, const std::string name) {references[i].name = name;}
    void setReferenceComment(int i, const std::string comment) {references[i].comment = comment;}
	bool sketchFileBySequence(FILE * file, ThreadPool<Sketch::SketchInput, Sketch::SketchOutput> * threadPool);
	void useThreadOutput(SketchOutput * output);
    void warnKmerSize(uint64_t lengthMax, const std::string & lengthMaxName, double randomChance, int kMin, int warningCount) const;
    bool writeToFile() const;
    int writeToCapnp(const char * file) const;
    
private:
    
    void createIndex();
    
    // Vettore dei riferimenti dello sketch 
    std::vector<Reference> references;

    robin_hood::unordered_map<std::string, int> referenceIndecesById;
    std::vector<std::vector<PositionHash>> positionHashesByReference;
    robin_hood::unordered_map<hash_t, std::vector<Locus>> lociByHash;
    
    Parameters parameters;
    double kmerSpace;
    std::string file;
};

void addMinHashes(MinHashHeap & minHashHeap, char * seq, uint64_t length, const Sketch::Parameters & parameters);
void getMinHashPositions(std::vector<Sketch::PositionHash> & loci, char * seq, uint32_t length, const Sketch::Parameters & parameters, int verbosity = 0);
bool hasSuffix(std::string const & whole, std::string const & suffix);
Sketch::SketchOutput * loadCapnp(Sketch::SketchInput * input);
void reverseComplement(const char * src, char * dest, int length);
void setAlphabetFromString(Sketch::Parameters & parameters, const char * characters);
void setMinHashesForReference(Sketch::Reference & reference, const MinHashHeap & hashes);
Sketch::SketchOutput * sketchFile(Sketch::SketchInput * input);
Sketch::SketchOutput * sketchSequence(Sketch::SketchInput * input);

int def(int fdSource, int fdDest, int level);
int inf(int fdSource, int fdDest);
void zerr(int ret);

#endif
