#ifndef INCLUDED_CommandTriangle
#define INCLUDED_CommandTriangle

#include "Command.h"
#include "CommandDistance.h"
#include "Sketch.h"

namespace mash {

class CommandTriangle : public Command
{
public:
    
    struct TriangleInput
    {
        TriangleInput(const Sketch & sketchNew, uint64_t indexNew, const Sketch::Parameters & parametersNew, double maxDistanceNew, double maxPValueNew, bool isFingerprintNew)
            :
            sketch(sketchNew),
            index(indexNew),
            parameters(parametersNew),
            maxDistance(maxDistanceNew),
            maxPValue(maxPValueNew),
            isFingerprint(isFingerprintNew)
            {}
        
        const Sketch & sketch;
        uint64_t index;
        const Sketch::Parameters & parameters;
        double maxDistance;
        double maxPValue;
        bool isFingerprint;
    };
    
    struct TriangleOutput
    {
        TriangleOutput(const Sketch & sketchNew, uint64_t indexNew)
            :
            sketch(sketchNew),
            index(indexNew)
        {
            pairs = new CommandDistance::CompareOutput::PairOutput[index];
        }
        
        ~TriangleOutput()
        {
            delete [] pairs;
        }
        
        const Sketch & sketch;
        uint64_t index;
        
        CommandDistance::CompareOutput::PairOutput * pairs;
    };
    
    CommandTriangle();


    
    int run() const override;
    
private:
    
    double pValueMax;
    bool comment;
    void writeOutput(TriangleOutput * output, bool comment, bool edge, double & pValuePeakToSet) const;

};

    CommandTriangle::TriangleOutput * compare(CommandTriangle::TriangleInput * input);
    void compareFingerprints(CommandDistance::CompareOutput::PairOutput * pair, const Sketch::Reference & ref1, const Sketch::Reference & ref2, uint64_t sketchSize, double maxDistance, double maxPValue);
    bool containsExtensionMSH(const std::vector<std::string>& strVec) ;
    bool containsExtensionTXT(const std::vector<std::string>& strVec) ;
} // namespace mash

#endif
