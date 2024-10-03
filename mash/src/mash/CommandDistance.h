#ifndef INCLUDED_CommandDistance
#define INCLUDED_CommandDistance

#include "Command.h"
#include "Sketch.h"
#include "SketchFingerPrint.h"
#include "HashList.h"

namespace mash {

class CommandDistance : public Command
{
public:
    
    struct CompareInput
    {
        CompareInput(const Sketch & sketchRefNew, const Sketch & sketchQueryNew, uint64_t indexRefNew, uint64_t indexQueryNew, uint64_t pairCountNew, const Sketch::Parameters & parametersNew, double maxDistanceNew, double maxPValueNew)
            :
            sketchRef(sketchRefNew),
            sketchQuery(sketchQueryNew),
            indexRef(indexRefNew),
            indexQuery(indexQueryNew),
            pairCount(pairCountNew),
            parameters(parametersNew),
            maxDistance(maxDistanceNew),
            maxPValue(maxPValueNew)
            {}
        
        const Sketch & sketchRef;
        const Sketch & sketchQuery;
        
        uint64_t indexRef;
        uint64_t indexQuery;
        uint64_t pairCount;
        
        const Sketch::Parameters & parameters;
        double maxDistance;
        double maxPValue;
    };
    
    struct CompareOutput
    {
        CompareOutput(const Sketch & sketchRefNew, const Sketch & sketchQueryNew, uint64_t indexRefNew, uint64_t indexQueryNew, uint64_t pairCountNew)
            :
            sketchRef(sketchRefNew),
            sketchQuery(sketchQueryNew),
            indexRef(indexRefNew),
            indexQuery(indexQueryNew),
            pairCount(pairCountNew)
        {
            pairs = new PairOutput[pairCount];
        }
        
        ~CompareOutput()
        {
            delete [] pairs;
        }
        
        struct PairOutput
        {
            uint64_t numer;
            uint64_t denom;
            double distance;
            double pValue;
            bool pass;
        };
        
        const Sketch & sketchRef;
        const Sketch & sketchQuery;
        
        uint64_t indexRef;
        uint64_t indexQuery;
        uint64_t pairCount;
        
        PairOutput * pairs;
    };
    

    
    struct CompareFingerPrintInput
    {
        CompareFingerPrintInput(const SketchFingerPrint & sketchRefNew, const SketchFingerPrint & sketchQueryNew, uint64_t indexRefNew, uint64_t indexQueryNew, uint64_t pairCountNew, const SketchFingerPrint::Parameters & parametersNew, double maxDistanceNew, double maxPValueNew)
            :
            sketchRef(sketchRefNew),
            sketchQuery(sketchQueryNew),
            indexRef(indexRefNew),
            indexQuery(indexQueryNew),
            pairCount(pairCountNew),
            parameters(parametersNew),
            maxDistance(maxDistanceNew),
            maxPValue(maxPValueNew)
            {}
        
        const SketchFingerPrint & sketchRef;
        const SketchFingerPrint & sketchQuery;
        
        uint64_t indexRef;
        uint64_t indexQuery;
        uint64_t pairCount;
        
        const SketchFingerPrint::Parameters & parameters;
        double maxDistance;
        double maxPValue;
    };
    
    struct CompareFingerPrintOutput
    {
        CompareFingerPrintOutput(const SketchFingerPrint & sketchRefNew, const SketchFingerPrint & sketchQueryNew, uint64_t indexRefNew, uint64_t indexQueryNew, uint64_t pairCountNew)
            :
            sketchRef(sketchRefNew),
            sketchQuery(sketchQueryNew),
            indexRef(indexRefNew),
            indexQuery(indexQueryNew),
            pairCount(pairCountNew)
        {
            pairs = new PairOutput[pairCount];
        }
        
        ~CompareFingerPrintOutput()
        {
            delete [] pairs;
        }
        
        struct PairOutput
        {
            uint64_t numer;
            uint64_t denom;
            double distance;
            double pValue;
            bool pass;
        };
        
        const SketchFingerPrint & sketchRef;
        const SketchFingerPrint & sketchQuery;
        
        uint64_t indexRef;
        uint64_t indexQuery;
        uint64_t pairCount;
        
        PairOutput * pairs;
    };
    



    CommandDistance();
    
    int run() const; // override

    int runFingerPrint() const;
    
private:
    
    void writeOutput(CompareOutput * output, bool table, bool comment) const;

    // ------------------------------------ FINGERPRINT ----------------------------------//

    void writeFingerPrintOutput(CompareFingerPrintOutput * output, bool table, bool comment) const;



    bool containsMSH(const std::vector<std::string>& strVec) const ;
    bool containsTXT(const std::vector<std::string>& strVec) const ;
};


// ------------------------------------ FINGERPRINT ----------------------------------//



// ------------------------------------ FINGERPRINT With Exact Similarity----------------------------------//

CommandDistance::CompareFingerPrintOutput * compareFingerPrintExcatSimilarity(CommandDistance::CompareFingerPrintInput * input);

void compareFingerPrintSketches(CommandDistance::CompareFingerPrintOutput::PairOutput* output, const HashList& hashesSortedRef, const HashList& hashesSortedQry, uint64_t sketchSize, int kmerSize, double kmerSpace, double maxDistance, double maxPValue, uint64_t& common, uint64_t& denom);

// ------------------------------------ FINGERPRINT With %% Similarity----------------------------------//

CommandDistance::CompareFingerPrintOutput* compareFingerPrintWithPercentageSimilarity(CommandDistance::CompareFingerPrintInput* input);

double jaccardSimilarityAndCommon(const std::vector<HashList>& set1, const std::vector<HashList>& set2, uint64_t& totalCommon, uint64_t& totalDenom);





int hashEquals64(uint64_t hash1, uint64_t hash2, uint64_t hash_size);
int hashEquals32(uint32_t hash1, uint32_t hash2, uint32_t hash_size);
int distanceBetweenHashLists(const HashList& list1, const HashList& list2);
bool areHashListsSimilar(const HashList& list1, const HashList& list2);


int calculateUnion(const std::vector<HashList>& largerSet, const std::vector<HashList>& smallerSet);
int calculateIntersection(const std::vector<HashList>& largerSet, const std::vector<HashList>& smallerSet);


// ------------------------------------ FINGERPRINT ----------------------------------//




CommandDistance::CompareOutput * compare(CommandDistance::CompareInput * input);

void compareSketches(CommandDistance::CompareOutput::PairOutput * output, const Sketch::Reference & refRef, const Sketch::Reference & refQry, uint64_t sketchSize, int kmerSize, double kmerSpace, double maxDistance, double maxPValue);


double pValue(uint64_t x, uint64_t lengthRef, uint64_t lengthQuery, double kmerSpace, uint64_t sketchSize);



} // namespace mash

#endif
