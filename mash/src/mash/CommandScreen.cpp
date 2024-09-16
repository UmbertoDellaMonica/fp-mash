#include "CommandScreen.h"
#include "CommandDistance.h" // for pvalue
#include "Sketch.h"
#include "kseq.h"
#include <iostream>
#include <zlib.h>
#include "ThreadPool.h"
#include <math.h>
#include "robin_hood.h"

#ifdef USE_BOOST
    #include <boost/math/distributions/binomial.hpp>
    using namespace::boost::math;
#else
    #include <gsl/gsl_cdf.h>
#endif

#define SET_BINARY_MODE(file)
KSEQ_INIT(gzFile, gzread)

using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::string;
using std::vector;

namespace mash {

CommandScreen::CommandScreen()
: Command()
{
    name = "screen";
    summary = "Determine whether query sequences are within a larger mixture of sequences.";
    description = "Determine how well query sequences are contained within a mixture of sequences. The queries must be formatted as a single Mash sketch file (.msh), created with the `mash sketch` command. The <mixture> files can be contigs or reads, in fasta or fastq, gzipped or not, e \"-\" can be given for <mixture> to read from standard input. The <mixture> sequences are assumed to be nucleotides, and will be 6-frame translated if the <queries> are amino acids. The output fields are [identity, shared-hashes, median-multiplicity, p-value, query-ID, query-comment], where median-multiplicity is computed for shared hashes, based on the number of observations of those hashes within the mixture.";
    argumentString = "<queries>.msh <mixture> [<mixture>] ...";
    
    useOption("help");
    useOption("threads");
    addOption("winning!", Option(Option::Boolean, "w", "", "Winner-takes-all strategy for identity estimates. After counting hashes for each query, hashes that appear in multiple queries will be removed from all except the one with the best identity (ties broken by larger query), and other identities will be reduced. This removes output redundancy, providing a rough compositional outline.", ""));
    addOption("identity", Option(Option::Number, "i", "Output", "Minimum identity to report. Inclusive unless set to zero, in which case only identities greater than zero (i.e. with at least one shared hash) will be reported. Set to -1 to output everything.", "0", -1., 1.));
    addOption("pvalue", Option(Option::Number, "v", "Output", "Maximum p-value to report.", "1.0", 0., 1.));
    addOption("saturation", Option(Option::Boolean, "s", "", "Include saturation curve in output. Each line will have an additional field representing the absolute number of k-mers seen at each Jaccard increase, formatted as a comma-separated list.", ""));
    addOption("fingerprint", Option(Option::Boolean, "fp", "", "Option about fingerprint, insert as argument[0] file about sketch file and other files as .txt in argument[1]", ""));

}

int CommandScreen::run() const
{
    if (arguments.size() < 2 || options.at("help").active)
    {
        print();
        return 0;
    }

    bool sat = options.at("saturation").active;
    double pValueMax = options.at("pvalue").getArgumentAsNumber();
    double identityMin = options.at("identity").getArgumentAsNumber();
    bool fingerprint = options.at("fingerprint").active;

    vector<string> refArgVector;
    refArgVector.push_back(arguments[0]);

    Sketch sketch;
    Sketch::Parameters parameters;

    sketch.initFromFiles(refArgVector, parameters);

    string alphabet;
    sketch.getAlphabetAsString(alphabet);
    setAlphabetFromString(parameters, alphabet.c_str());

    parameters.parallelism = options.at("threads").getArgumentAsNumber();
    parameters.kmerSize = sketch.getKmerSize();
    parameters.noncanonical = sketch.getNoncanonical();
    parameters.use64 = sketch.getUse64();
    parameters.preserveCase = sketch.getPreserveCase();
    parameters.seed = sketch.getHashSeed();
    parameters.minHashesPerWindow = sketch.getMinHashesPerWindow();

    HashTable hashTable;
    robin_hood::unordered_map<uint64_t, std::atomic<uint32_t>> hashCounts;
    robin_hood::unordered_map<uint64_t, list<uint32_t>> saturationByIndex;

    cerr << "Loading " << arguments[0] << "..." << endl;

    for (int i = 0; i < sketch.getReferenceCount(); i++)
    {
        const HashList &hashes = sketch.getReference(i).hashesSorted;

        for (int j = 0; j < hashes.size(); j++)
        {
            uint64_t hash = hashes.get64() ? hashes.at(j).hash64 : hashes.at(j).hash32;

            if (hashTable.count(hash) == 0)
            {
                hashCounts[hash] = 0;
            }

            hashTable[hash].insert(i);
        }
    }

    cerr << "   " << hashTable.size() << " distinct hashes." << endl;

    // Load the query file
    vector<string> queryArgVector;
    queryArgVector.push_back(arguments[1]);

    /**
     * Se utilizzo l'opzione -fp , allora devo stabilire che i file vengono caricati secondo la funzione di initFromFingerprint()
     * 
     *  Altrimenti -> se ho dei file normali posso benissimamente utilizzare initFromFiles()
     */

    Sketch querySketch;

    if(fingerprint){
    
        querySketch.initFromFingerprints(queryArgVector,parameters);
    
    }else{
    
        querySketch.initFromFiles(queryArgVector, parameters);

    }
    cerr << "Loading " << arguments[1] << " as query..." << endl;

    uint64_t setSize = hashTable.size();
    vector<uint64_t> shared(querySketch.getReferenceCount(), 0);
    vector<vector<uint64_t>> depths(querySketch.getReferenceCount());

    for (int i = 0; i < querySketch.getReferenceCount(); i++)
    {
        const HashList &queryHashes = querySketch.getReference(i).hashesSorted;

        for (int j = 0; j < queryHashes.size(); j++)
        {
            uint64_t queryHash = queryHashes.get64() ? queryHashes.at(j).hash64 : queryHashes.at(j).hash32;

            if (hashTable.count(queryHash))
            {
                shared[i]++;
                depths[i].push_back(hashCounts[queryHash]);
                 if (sat)
                 {
                     saturationByIndex[i].push_back(0);
                 }
            }
        }
    }

    if (options.at("winning!").active)
    {
        cerr << "Reallocating to winners..." << endl;
        double *scores = new double[sketch.getReferenceCount()];

        for (int i = 0; i < sketch.getReferenceCount(); i++)
        {
            scores[i] = estimateIdentity(shared[i], sketch.getReference(i).hashesSorted.size(), parameters.kmerSize, sketch.getKmerSpace());
        }

        memset(shared.data(), 0, sizeof(uint64_t) * sketch.getReferenceCount());

        for (int i = 0; i < sketch.getReferenceCount(); i++)
        {
            depths[i].clear();
        }

        for (auto &pair : hashTable)
        {
            if (hashCounts.count(pair.first) == 0 || hashCounts[pair.first] < 1)
            {
                continue;
            }

            const auto &indices = pair.second;
            double maxScore = 0;
            uint64_t maxLength = 0;
            uint64_t maxIndex;

            for (auto k = indices.begin(); k != indices.end(); k++)
            {
                if (scores[*k] > maxScore)
                {
                    maxScore = scores[*k];
                    maxIndex = *k;
                    maxLength = sketch.getReference(*k).length;
                }
                else if (scores[*k] == maxScore && sketch.getReference(*k).length > maxLength)
                {
                    maxIndex = *k;
                    maxLength = sketch.getReference(*k).length;
                }
            }

            shared[maxIndex]++;
            depths[maxIndex].push_back(hashCounts[pair.first]);
        }

        delete[] scores;
    }

    cerr << "Computing coverage medians..." << endl;

    // TODO : La computazione si ferma qui ------- Verificare i prossimi passaggi con la fingerprint 

    for (int i = 0; i < sketch.getReferenceCount(); i++)
    {
        sort(depths[i].begin(), depths[i].end());
    }

    cerr << "Writing output..." << endl;

    for (int i = 0; i < querySketch.getReferenceCount(); i++)
    {
            cerr << "Sono qui !..." << endl;
        if (shared[i] != 0 || identityMin < 0.0)
        {
            double identity = estimateIdentity(shared[i], querySketch.getReference(i).hashesSorted.size(), parameters.kmerSize, sketch.getKmerSpace());

            if (identity < identityMin)
            {
                continue;
            }

            double pValue = pValueWithin(shared[i], setSize, sketch.getKmerSpace(), querySketch.getReference(i).hashesSorted.size());

            if (pValue > pValueMax)
            {
                continue;
            }

            cout << identity << '\t' << shared[i] << '/' << querySketch.getReference(i).hashesSorted.size() << '\t'
                 << (shared[i] > 0 ? depths[i].at(shared[i] / 2) : 0) << '\t' << pValue << '\t' << querySketch.getReference(i).name << '\t' << querySketch.getReference(i).comment;

             if (sat)
             {
                 cout << '\t';

                 for (auto j = saturationByIndex.at(i).begin(); j != saturationByIndex.at(i).end(); j++)
                 {
                     if (j != saturationByIndex.at(i).begin())
                     {
                         cout << ',';
                     }

                     cout << *j;
                 }
             }

            cout << endl;
        }
    }

    return 0;
}

double estimateIdentity(uint64_t common, uint64_t denom, int kmerSize, double kmerSpace)
{
    double identity;
    double jaccard = double(common) / denom;

    if (common == denom)
    {
        identity = 1.;
    }
    else if (common == 0)
    {
        identity = 0.;
    }
    else
    {
        identity = pow(jaccard, 1. / kmerSize);
    }

    return identity;
}

CommandScreen::HashOutput * hashSequence(CommandScreen::HashInput * input)
{
    CommandScreen::HashOutput * output = new CommandScreen::HashOutput(input->minHashHeap);

    int l = input->length;
    bool trans = input->trans;

    bool use64 = input->parameters.use64;
    uint32_t seed = input->parameters.seed;
    int kmerSize = input->parameters.kmerSize;
    bool noncanonical = input->parameters.noncanonical;

    char * seq = input->seq;

    // Converti in maiuscolo
    for (uint64_t i = 0; i < l; i++)
    {
        if (!input->parameters.preserveCase && seq[i] > 96 && seq[i] < 123)
        {
            seq[i] -= 32;
        }
    }

    char * seqRev;

    if (!noncanonical || trans)
    {
        seqRev = new char[l];
        reverseComplement(seq, seqRev, l);
    }

    for (int i = 0; i < (trans ? 6 : 1); i++)
    {
        bool useRevComp = false;
        int frame = i % 3;
        bool rev = i > 2;

        int lenTrans = (l - frame) / 3;

        char * seqTrans;

        if (trans)
        {
            seqTrans = new char[lenTrans];
            translate((rev ? seqRev : seq) + frame, seqTrans, lenTrans);
        }

        int64_t lastGood = -1;
        int length = trans ? lenTrans : l;

        for (int j = 0; j < length - kmerSize + 1; j++)
        {
            while (lastGood < j + kmerSize - 1 && lastGood < length)
            {
                lastGood++;

                if (trans ? (seqTrans[lastGood] == '*') : (!input->parameters.alphabet[seq[lastGood]]))
                {
                    j = lastGood + 1;
                }
            }

            if (j > length - kmerSize)
            {
                break;
            }

            const char * kmer;

            if (trans)
            {
                kmer = seqTrans + j;
            }
            else
            {
                const char * kmer_fwd = seq + j;
                const char * kmer_rev = seqRev + length - j - kmerSize;
                kmer = (noncanonical || memcmp(kmer_fwd, kmer_rev, kmerSize) <= 0) ? kmer_fwd : kmer_rev;
            }

            hash_u hash = getHash(kmer, kmerSize, seed, use64);
            uint64_t key = use64 ? hash.hash64 : hash.hash32;

            // Messaggio di debug
            cerr << "Comparing Hash: " << key << " in hashCounts" << endl;

            if (input->hashCounts.count(key) == 1)
            {
                input->hashCounts[key]++;
            }
        }

        if (trans)
        {
            delete [] seqTrans;
        }
    }

    if (!noncanonical || trans)
    {
        delete [] seqRev;
    }

    return output;
}

double pValueWithin(uint64_t x, uint64_t setSize, double kmerSpace, uint64_t sketchSize)
{
    if (x == 0)
    {
        return 1.0;
    }

    double r = double(setSize) / kmerSpace;

    if (r < 0.0 || r > 1.0)
    {
        cerr << "ERROR: Invalid probability value r = " << r << ". Adjusting to be within [0, 1]." << endl;
        r = std::max(0.0, std::min(1.0, r));
    }

#ifdef USE_BOOST
    return cdf(complement(binomial(sketchSize, r), x - 1));
#else
    return gsl_cdf_binomial_Q(x - 1, r, sketchSize);
#endif
}

void translate(const char * src, char * dst, uint64_t len)
{
    for (uint64_t n = 0, a = 0; a < len; a++, n += 3)
    {
        dst[a] = aaFromCodon(src + n);
    }
}

char aaFromCodon(const char * codon)
{
    string str(codon, 3);
    char aa = '*';

    switch (codon[0])
    {
    case 'A':
        switch (codon[1])
        {
        case 'A':
            switch (codon[2])
            {
                case 'A': aa = 'K'; break;
                case 'C': aa = 'N'; break;
                case 'G': aa = 'K'; break;
                case 'T': aa = 'N'; break;
            }
            break;
        case 'C':
            switch (codon[2])
            {
                case 'A': aa = 'T'; break;
                case 'C': aa = 'T'; break;
                case 'G': aa = 'T'; break;
                case 'T': aa = 'T'; break;
            }
            break;
        case 'G':
            switch (codon[2])
            {
                case 'A': aa = 'R'; break;
                case 'C': aa = 'S'; break;
                case 'G': aa = 'R'; break;
                case 'T': aa = 'S'; break;
            }
            break;
        case 'T':
            switch (codon[2])
            {
                case 'A': aa = 'I'; break;
                case 'C': aa = 'I'; break;
                case 'G': aa = 'M'; break;
                case 'T': aa = 'I'; break;
            }
            break;
        }
        break;
    case 'C':
        switch (codon[1])
        {
        case 'A':
            switch (codon[2])
            {
                case 'A': aa = 'Q'; break;
                case 'C': aa = 'H'; break;
                case 'G': aa = 'Q'; break;
                case 'T': aa = 'H'; break;
            }
            break;
        case 'C':
            switch (codon[2])
            {
                case 'A': aa = 'P'; break;
                case 'C': aa = 'P'; break;
                case 'G': aa = 'P'; break;
                case 'T': aa = 'P'; break;
            }
            break;
        case 'G':
            switch (codon[2])
            {
                case 'A': aa = 'R'; break;
                case 'C': aa = 'R'; break;
                case 'G': aa = 'R'; break;
                case 'T': aa = 'R'; break;
            }
            break;
        case 'T':
            switch (codon[2])
            {
                case 'A': aa = 'L'; break;
                case 'C': aa = 'L'; break;
                case 'G': aa = 'L'; break;
                case 'T': aa = 'L'; break;
            }
            break;
        }
        break;
    case 'G':
        switch (codon[1])
        {
        case 'A':
            switch (codon[2])
            {
                case 'A': aa = 'E'; break;
                case 'C': aa = 'D'; break;
                case 'G': aa = 'E'; break;
                case 'T': aa = 'D'; break;
            }
            break;
        case 'C':
            switch (codon[2])
            {
                case 'A': aa = 'A'; break;
                case 'C': aa = 'A'; break;
                case 'G': aa = 'A'; break;
                case 'T': aa = 'A'; break;
            }
            break;
        case 'G':
            switch (codon[2])
            {
                case 'A': aa = 'G'; break;
                case 'C': aa = 'G'; break;
                case 'G': aa = 'G'; break;
                case 'T': aa = 'G'; break;
            }
            break;
        case 'T':
            switch (codon[2])
            {
                case 'A': aa = 'V'; break;
                case 'C': aa = 'V'; break;
                case 'G': aa = 'V'; break;
                case 'T': aa = 'V'; break;
            }
            break;
        }
        break;
    case 'T':
        switch (codon[1])
        {
        case 'A':
            switch (codon[2])
            {
                case 'A': aa = '*'; break;
                case 'C': aa = 'Y'; break;
                case 'G': aa = '*'; break;
                case 'T': aa = 'Y'; break;
            }
            break;
        case 'C':
            switch (codon[2])
            {
                case 'A': aa = 'S'; break;
                case 'C': aa = 'S'; break;
                case 'G': aa = 'S'; break;
                case 'T': aa = 'S'; break;
            }
            break;
        case 'G':
            switch (codon[2])
            {
                case 'A': aa = '*'; break;
                case 'C': aa = 'C'; break;
                case 'G': aa = 'W'; break;
                case 'T': aa = 'C'; break;
            }
            break;
        case 'T':
            switch (codon[2])
            {
                case 'A': aa = 'L'; break;
                case 'C': aa = 'F'; break;
                case 'G': aa = 'L'; break;
                case 'T': aa = 'F'; break;
            }
            break;
        }
        break;
    }

    return aa;
}

void useThreadOutput(CommandScreen::HashOutput * output, robin_hood::unordered_set<MinHashHeap *> & minHashHeaps)
{
    minHashHeaps.emplace(output->minHashHeap);
    delete output;
}

} // namespace mash
