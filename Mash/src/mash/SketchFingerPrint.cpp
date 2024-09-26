// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "SketchFingerprint.h"
#include <unistd.h>
#include <zlib.h>
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <map>
#include "kseq.h"
#include "MurmurHash3.h"
#include <assert.h>
#include <queue>
#include <deque>
#include <set>
#include "Command.h" // TEMP for column printing
#include <sys/stat.h>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <sys/mman.h>
#include <math.h>
#include <list>
#include <string.h>
#include <fstream>
#include <sstream>
#include <numeric>
#include "hash.h"




#define SET_BINARY_MODE(file)
#define CHUNK 16384
#define LIMIT_READ_FINGERPRINT 1000000
KSEQ_INIT(gzFile, gzread)

using namespace std;


void SketchFingerPrint::getAlphabetAsString(string & alphabet) const
{
	for ( int i = 0; i < 256; i++ )
	{
		if ( parameters.alphabet[i] )
		{
			alphabet.append(1, i);
		}
	}
}


void SketchFingerPrint::initFromFingerprintsWithSingleHash(const vector<string> &files, const Parameters &parametersNew)
{
    /*parameters = parametersNew;
    
    int counterLine = 0;
    robin_hood::unordered_set<string> processedIDs; // Usare unordered_set per ID unici
                                                     // Vettore per memorizzare le reference finali
    string lastID = ""; // Memorizza l'ultimo ID letto

    cout << "Initializing from fingerprints..." << endl;

    for (const string &file : files)
    {
        cout << "Processing file: " << file << endl;

        ifstream inputFile(file);

        if (!inputFile)
        {
            cerr << "ERROR: Could not open fingerprint file " << file << " for reading." << endl;
            exit(1);
        }

        string line;
        Reference* currentReference = nullptr; // Puntatore alla reference corrente

        while (getline(inputFile, line) && counterLine < LIMIT_READ_FINGERPRINT)
        {
            //cout << "Reading line: " << counterLine + 1 << endl;
            counterLine++;

            vector<uint64_t> fingerprint;

            istringstream ss(line);
            
            uint64_t number;
            string id;
            ss >> id;

            //cout << "Processing ID: " << id << endl;

            while (ss >> number)
            {
                fingerprint.push_back(number);
                if (ss.peek() == ' ') ss.ignore();
            }

            // Verifica se l'ID è diverso dall'ultimo ID
            if (id != lastID)
            {
                // Nuovo ID, crea una nuova reference
                if (currentReference != nullptr)
                {
                    //cout << "Adding reference for ID: " << lastID << endl;
                    // Aggiungi la reference al vettore
                    references.push_back(*currentReference);
                    delete currentReference;
                }

                currentReference = new Reference;
                currentReference->id = id; // Assegna l'ID estratto alla struttura Reference
                currentReference->length = fingerprint.size(); // Numero di elementi nel fingerprint
                currentReference->name = ""+id;
                currentReference->comment = "FingerPrint : " + currentReference->id;

                // Aggiungi l'ID al set di ID processati
                processedIDs.insert(id);

                // Aggiorna l'ultimo ID
                lastID = id;

                //cout << "Created new reference for ID: " << id << endl;
            }

            // Calcola Hash in base64 
            hash_u hash = getHashFingerPrint(fingerprint, fingerprint.size() * sizeof(uint64_t), parameters.seed, parameters.use64);
            currentReference->hashesSorted.add(hash); // Usa il metodo add
            currentReference->length = currentReference->length+fingerprint.size();

            //cout << "Added hash for ID: " << id << endl;
        }

        if (currentReference != nullptr)
        {
            //cout << "Adding last reference for file: " << file << endl;
            // Aggiungi l'ultima reference processata al vettore
            references.push_back(*currentReference);
            delete currentReference;
        }
    }

    //cout << "Creating index..." << endl;
    createIndex();
    cout << "Initialization complete." << endl;*/

}


void SketchFingerPrint::initFromFingerprints(const vector<string> &files, const Parameters &parametersNew)
{

    parameters = parametersNew;
    int counterLine = 0;
    robin_hood::unordered_set<string> processedIDs;
    string lastID = "";
    cout << "Initializing from fingerprints..." << endl;

    for (const string &file : files)
    {
        cout << "Processing file: " << file << endl;
        ifstream inputFile(file);
        if (!inputFile)
        {
            cerr << "ERROR: Could not open fingerprint file " << file << " for reading." << endl;
            exit(1);
        }

        string line;
        Reference* currentReference = nullptr;


        while (getline(inputFile, line) && counterLine < LIMIT_READ_FINGERPRINT)
        {
            counterLine++;
            vector<uint64_t> fingerprint;
            istringstream ss(line);
            uint64_t number;
            string id;
            ss >> id;

            while (ss >> number)
            {
                fingerprint.push_back(number);
                if (ss.peek() == ' ') ss.ignore();
            }

            // Se l'ID è nuovo, crea una nuova reference
            if (id != lastID)
            {
                if (currentReference != nullptr)
                {
                    references.push_back(*currentReference);
                    delete currentReference;
                }

                currentReference = new Reference;
                currentReference->id = id;
                currentReference->name = id;
                currentReference->comment = "FingerPrint : " + currentReference->id;
                currentReference->use64 = true;
                lastID = id;
            }


            // Calcola hash e inserisci solo valori diversi da 0

            HashList  hashListSubSketch ; 
            for (uint64_t num : fingerprint)
            {
                hash_u hash = getHashNumber(&num, sizeof(uint64_t), parameters.seed, parameters.use64);
                if (hash.hash64 != 0) // Aggiungi solo hash non nulli
                {
                    // Aggiungi il valore all'hash list 
                    hashListSubSketch.add(hash);
                }
            }

            currentReference->subSketch_list.push_back(hashListSubSketch);
            currentReference->length += 1; 
        }

        if (currentReference != nullptr)
        {
            references.push_back(*currentReference);
            delete currentReference;
        }
    }

    createIndexFingerPrint();
    cout << "Initialization complete." << endl;
}


int SketchFingerPrint::getMinKmerSize(uint64_t reference) const
{
	return ceil(log(references[reference].length * (1 - parameters.warning) / parameters.warning) / log(parameters.alphabetSize));
}

double SketchFingerPrint::getRandomKmerChance(uint64_t reference) const
{
	return 1. / (kmerSpace / references[reference].length + 1.);
}

void SketchFingerPrint::getReferenceHistogram(uint64_t index, map<uint32_t, uint64_t> & histogram) const
{
	const Reference & reference = references.at(index);
	
	histogram.clear();
	
	for ( uint64_t i = 0; i < reference.counts.size(); i++ )
	{
		uint32_t count = reference.counts.at(i);
		
		if ( histogram.count(count) == 0 )
		{
			histogram[count] = 1;
		}
		else
		{
			histogram[count] = histogram.at(count) + 1;
		}
	}
}

uint64_t SketchFingerPrint::getReferenceIndex(string id) const
{
    if ( referenceIndecesById.count(id) == 1 )
    {
        return referenceIndecesById.at(id);
    }
    else
    {
        return -1;
    }
}



int SketchFingerPrint::initFromFingerPrintFiles(const vector<string> & files, const Parameters & parametersNew, int verbosity, bool enforceParameters, bool contain)
{
    parameters = parametersNew;
    
	ThreadPool<SketchFingerPrint::SketchInput, SketchFingerPrint::SketchOutput> threadPool(0, parameters.parallelism);
	
    for ( int i = 0; i < files.size(); i++ )
    {
        bool isSketch = hasSuffixFingerPrint(files[i], parameters.windowed ? suffixFingerPrintSketchWindowed : suffixFingerPrintSketch);
        
        if ( isSketch )
        {
			// init header to check params
			//
			SketchFingerPrint sketchTest;
			sketchTest.initParametersFingerPrintsFromCapnp(files[i].c_str());
			
        	if ( i == 0 && ! enforceParameters )
        	{
        		initParametersFingerPrintsFromCapnp(files[i].c_str());
        	}
        	
            string alphabet;
            string alphabetTest;
            
            getAlphabetAsString(alphabet);
            sketchTest.getAlphabetAsString(alphabetTest);
            
            if ( alphabet != alphabetTest )
            {
            	cerr << "\nWARNING: The sketch file " << files[i] << " has different alphabet (" << alphabetTest << ") than the current alphabet (" << alphabet << "). This file will be skipped." << endl << endl;
            	continue;
            }
			
            if ( sketchTest.getHashSeed() != parameters.seed )
            {
				cerr << "\nWARNING: The sketch " << files[i] << " has a seed size (" << sketchTest.getHashSeed() << ") that does not match the current seed (" << parameters.seed << "). This file will be skipped." << endl << endl;
				continue;
            }
			if ( sketchTest.getKmerSize() != parameters.kmerSize )
			{
				cerr << "\nWARNING: The sketch " << files[i] << " has a kmer size (" << sketchTest.getKmerSize() << ") that does not match the current kmer size (" << parameters.kmerSize << "). This file will be skipped." << endl << endl;
				continue;
			}
			
            if ( ! contain && sketchTest.getMinHashesPerWindow() < parameters.minHashesPerWindow )
            {
                cerr << "\nWARNING: The sketch file " << files[i] << " has a target sketch size (" << sketchTest.getMinHashesPerWindow() << ") that is smaller than the current sketch size (" << parameters.minHashesPerWindow << "). This file will be skipped." << endl << endl;
                continue;
            }
            
			if ( sketchTest.getNoncanonical() != parameters.noncanonical )
			{
				cerr << "\nWARNING: The sketch file " << files[i] << " is " << (sketchTest.getNoncanonical() ? "noncanonical" : "canonical") << ", which is incompatible with the current setting. This file will be skipped." << endl << endl;
				continue;
			}
            
            if ( sketchTest.getMinHashesPerWindow() > parameters.minHashesPerWindow )
            {
                cerr << "\nWARNING: The sketch file " << files[i] << " has a target sketch size (" << sketchTest.getMinHashesPerWindow() << ") that is larger than the current sketch size (" << parameters.minHashesPerWindow << "). Its sketches will be reduced." << endl << endl;
            }
            // init fully
            //
            vector<string> file;
            file.push_back(files[i]);
			threadPool.runWhenThreadAvailable(new SketchInput(file, 0, 0, "", "", parameters), loadCapnpFingerPrint);
        }
        else
		{
			FILE * inStream;
		
			if ( files[i] == "-" )
			{
				if ( verbosity > 0 )
				{
					cerr << "Sketching from stdin..." << endl;
				}
			
				inStream = stdin;
			}
			else
			{
				if ( verbosity > 0 )
				{
					cerr << "Sketching " << files[i] << "..." << endl;
				}
			
				inStream = fopen(files[i].c_str(), "r");
			
				if ( inStream == NULL )
				{
					cerr << "ERROR: could not open " << files[i] << " for reading." << endl;
					exit(1);
				}
			}
		
			if ( parameters.concatenated )
			{
				if ( files[i] != "-" )
				{
					fclose(inStream);
				}
				
				vector<string> file;
				file.push_back(files[i]);
				threadPool.runWhenThreadAvailable(new SketchInput(file, 0, 0, "", "", parameters), sketchFile);
			}
			else
			{
				if ( ! sketchFileBySequence(inStream, &threadPool) )
				{
					cerr << "\nERROR: reading " << files[i] << "." << endl;
					exit(1);
				}
			
				fclose(inStream);
			}
		}
			
		while ( threadPool.outputAvailable() )
		{
			useThreadOutput(threadPool.popOutputWhenAvailable());
		}
    }
    
	while ( threadPool.running() )
	{
		useThreadOutput(threadPool.popOutputWhenAvailable());
	}
	
    /*
    printf("\nCombined hash table:\n\n");
    
    for ( LociByHash_umap::iterator i = lociByHash.begin(); i != lociByHash.end(); i++ )
    {
        printf("Hash %u:\n", i->first);
        
        for ( int j = 0; j < i->second.size(); j++ )
        {
            printf("   Seq: %d\tPos: %d\n", i->second.at(j).sequence, i->second.at(j).position);
        }
    }
    */
    
    createIndexFingerPrint();
    
    return 0;
}



uint64_t SketchFingerPrint::initParametersFingerPrintsFromCapnp(const char * file)
{
    int fd = open(file, O_RDONLY);
    
    if ( fd < 0 )
    {
        cerr << "ERROR: could not open \"" << file << "\" for reading." << endl;
        exit(1);
    }
    
    struct stat fileInfo;
    
    if ( stat(file, &fileInfo) == -1 )
    {
        cerr << "ERROR: could not get file stats for \"" << file << "\"." << endl;
        exit(1);
    }
    
    void * data = mmap(NULL, fileInfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    if (data == MAP_FAILED)
    {
        uint64_t fileSize = fileInfo.st_size;
        std::cerr << "Error: could not memory-map file " << file << " of size " << fileSize;
        std::cerr << std::endl;
        exit(1);
    }

    capnp::ReaderOptions readerOptions;
    
    readerOptions.traversalLimitInWords = 1000000000000;
    readerOptions.nestingLimit = 1000000;
    
    //capnp::StreamFdMessageReader * message = new capnp::StreamFdMessageReader(fd, readerOptions);
    capnp::FlatArrayMessageReader * message = new capnp::FlatArrayMessageReader(kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word *>(data), fileInfo.st_size / sizeof(capnp::word)), readerOptions);
    capnp::MinHashFingerPrint::Reader reader = message->getRoot<capnp::MinHashFingerPrint>();
    
    parameters.kmerSize = reader.getKmerSize();
    parameters.error = reader.getError();
    parameters.minHashesPerWindow = reader.getMinHashesPerWindow();
    parameters.windowSize = reader.getWindowSize();
    parameters.concatenated = reader.getConcatenated();
    parameters.noncanonical = reader.getNoncanonical();
   	parameters.preserveCase = reader.getPreserveCase();

    capnp::MinHashFingerPrint::ReferenceList::Reader referenceListReader;
    if(reader.getReferenceList().getReferences().size() >0 ){
        
        
        referenceListReader = reader.getReferenceList();

    
    }else{

     referenceListReader = reader.getReferenceListOld();
    }
    
    capnp::List<capnp::MinHashFingerPrint::ReferenceList::Reference>::Reader referencesReader = referenceListReader.getReferences();
    uint64_t referenceCount = referencesReader.size();
    

    parameters.counts = referencesReader[0].hasCounts32();
   	parameters.seed = reader.getHashSeed();
    
    if ( reader.hasAlphabet() )
    {
    	setAlphabetFingerPrintFromString(parameters, reader.getAlphabet().cStr());
    }
    else
    {
    	setAlphabetFingerPrintFromString(parameters, alphabetNucleotideFingerPrint);
    }
	close(fd);
	
	try
	{
		delete message;
	}
	catch (exception e) {}
	
	return referenceCount;
}


bool SketchFingerPrint::sketchFileBySequence(FILE * file, ThreadPool<SketchFingerPrint::SketchInput, SketchFingerPrint::SketchOutput> * threadPool)
{
	gzFile fp = gzdopen(fileno(file), "r");
	kseq_t *seq = kseq_init(fp);
	
    int l;
    int count = 0;
	bool skipped = false;
	
	while ((l = kseq_read(seq)) >= 0)
	{
		if ( l < parameters.kmerSize )
		{
			skipped = true;
			continue;
		}
		
		//if ( verbosity > 0 && parameters.windowed ) cout << '>' << seq->name.s << " (" << l << "nt)" << endl << endl;
		//if (seq->comment.l) printf("comment: %s\n", seq->comment.s);
		//printf("seq: %s\n", seq->seq.s);
		//if (seq->qual.l) printf("qual: %s\n", seq->qual.s);
		
		// buffer this out since kseq will overwrite (SketchInput will delete)
		//
		char * seqCopy = new char[l];
		//
		memcpy(seqCopy, seq->seq.s, l);
		
		threadPool->runWhenThreadAvailable(new SketchInput(vector<string>(), seqCopy, l, string(seq->name.s, seq->name.l), string(seq->comment.s, seq->comment.l), parameters), sketchSequence);
		
		while ( threadPool->outputAvailable() )
		{
			useThreadOutput(threadPool->popOutputWhenAvailable());
		}
    	
		count++;
	}
	
	if (  l != -1 )
	{
		return false;
	}
	
	return true;
}


void SketchFingerPrint::useThreadOutput(SketchOutput * output)
{
	references.insert(references.end(), output->references.begin(), output->references.end());
	//positionHashesByReference.insert(positionHashesByReference.end(), output->positionHashesByReference.begin(), output->positionHashesByReference.end());
	delete output;
}


int SketchFingerPrint::writeToCapnpFingerPrint(const char * file) const {
    
    
    cout << "Opening file: " << file << endl;
    int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC , 0644);

    cout<< "FILE Name : "<< file<< endl;


    if (fd < 0)
    {
        cerr << "ERROR: could not open " << file << " for writing.\n";
        exit(1);
    }

    capnp::MallocMessageBuilder message;
    capnp::MinHashFingerPrint::Builder builder = message.initRoot<capnp::MinHashFingerPrint>();

    capnp::MinHashFingerPrint::ReferenceList::Builder referenceListBuilder = (parameters.seed == 42 ? builder.initReferenceListOld() : builder.initReferenceList());


    capnp::List<capnp::MinHashFingerPrint::ReferenceList::Reference>::Builder referencesBuilder = referenceListBuilder.initReferences(references.size());


    for (uint64_t i = 0; i < references.size(); i++){   
        
        // i-esimo Reference : Builder
        capnp::MinHashFingerPrint::ReferenceList::Reference::Builder referenceBuilder = referencesBuilder[i];

        referenceBuilder.setId(references[i].id);
        
        referenceBuilder.setName(references[i].name);
        
        referenceBuilder.setComment(references[i].comment);

        referenceBuilder.setLength64(references[i].length);
        
        referenceBuilder.setUseHash64(references[i].use64);
        

        if(references[i].use64 == true){

            const vector<HashList> subSketchVectorHashList = references[i].subSketch_list;
            // vector HashList : Builder
            capnp::List<capnp::MinHashFingerPrint::HashList>::Builder subSketchVectorHashListBuilder = referenceBuilder.initSubSketchList(subSketchVectorHashList.size());
            
            
            
            for (uint64_t j = 0; j < subSketchVectorHashList.size(); j++){

                const HashList & subSketchHashList = subSketchVectorHashList.at(j);


                capnp::MinHashFingerPrint::HashList::Builder hashListBuilder = subSketchVectorHashListBuilder[j];

                
                capnp::List<uint64_t>::Builder hashes64Builder = hashListBuilder.initHashList64(subSketchHashList.size());


                for (uint64_t k = 0; k != subSketchHashList.size(); k++)
                {
                    hashes64Builder.set(k, subSketchHashList.at(k).hash64);
                }
                
            }
            

        }else{



            const vector<HashList> subSketchVectorHashList = references[i].subSketch_list;
            // vector HashList : Builder
            capnp::List<capnp::MinHashFingerPrint::HashList>::Builder subSketchVectorHashListBuilder = referenceBuilder.initSubSketchList(subSketchVectorHashList.size());
            
            for (uint64_t j = 0; j < subSketchVectorHashList.size(); j++){

                auto & subSketchHashList = subSketchVectorHashList.at(j);

                capnp::MinHashFingerPrint::HashList::Builder hashListBuilder = subSketchVectorHashListBuilder[j];

                
                capnp::List<uint32_t>::Builder hashes32Builder = hashListBuilder.initHashList32(subSketchHashList.size());


                for (uint64_t k = 0; k != subSketchHashList.size(); k++)
                {
                    
                    hashes32Builder.set(k, subSketchHashList.at(k).hash32);
                }
                
            }
        }
       
    }

    cout << "Setting general parameters" << endl;
    builder.setKmerSize(parameters.kmerSize);
    builder.setHashSeed(parameters.seed);
    builder.setError(parameters.error);
    builder.setMinHashesPerWindow(parameters.minHashesPerWindow);
    builder.setWindowSize(parameters.windowSize);
    builder.setConcatenated(parameters.concatenated);
    builder.setNoncanonical(parameters.noncanonical);
    builder.setPreserveCase(parameters.preserveCase);

    string alphabet;
    getAlphabetAsString(alphabet);
    builder.setAlphabet(alphabet);

    cout << "Writing message to file" << endl;
    capnp::writeMessageToFd(fd, message);
    close(fd);

    cout << "Finished writing to file" << endl;

    return 0;
}





void SketchFingerPrint::createIndexFingerPrint()
{

    if (references.size() == 0)
    {
        cerr << "ERROR: references is empty." << endl;
        return;
    }

    // Popola l'indice delle referenze basato su ID
    for (int i = 0; i < references.size(); i++)
    {
        if (references[i].id.empty())
        {
            cerr << "ERROR: references[" << i << "].id is empty." << endl;
            return;
        }

        referenceIndecesById[references[i].id] = i;
    }

    // Controlla che alphabetSize e kmerSize siano validi
    if (parameters.alphabetSize <= 0 || parameters.kmerSize <= 0)
    {
        cerr << "ERROR: alphabetSize and kmerSize must be greater than 0." << endl;
        return;
    }

    // Calcola lo spazio dei k-mer
    kmerSpace = pow(parameters.alphabetSize, parameters.kmerSize);

}









void addMinHashes(MinHashHeap & minHashHeap, char * seq, uint64_t length, const SketchFingerPrint::Parameters & parameters)
{
    int kmerSize = parameters.kmerSize;
    uint64_t mins = parameters.minHashesPerWindow;
    bool noncanonical = parameters.noncanonical;
    
    // Determine the 'mins' smallest hashes, including those already provided
    // (potentially replacing them). This allows min-hash sets across multiple
    // sequences to be determined.
    
    // uppercase TODO: alphabets?
    //
    for ( uint64_t i = 0; i < length; i++ )
    {
        if ( ! parameters.preserveCase && seq[i] > 96 && seq[i] < 123 )
        {
            seq[i] -= 32;
        }
    }
    
    char * seqRev;
    
    if ( ! noncanonical )
    {
    	seqRev = new char[length];
        reverseFingerPrintComplement(seq, seqRev, length);
    }
    
    uint64_t j = 0;
    
    for ( uint64_t i = 0; i < length - kmerSize + 1; i++ )
    {
		// repeatedly skip kmers with bad characters
		//
		bool bad = false;
		//
		for ( ; j < i + kmerSize && i + kmerSize <= length; j++ )
		{
			if ( ! parameters.alphabet[seq[j]] )
			{
				i = j++; // skip to past the bad character
				bad = true;
				break;
			}
		}
		//
		if ( bad )
		{
			continue;
		}
		//	
		if ( i + kmerSize > length )
		{
			// skipped to end
			break;
		}
            
        const char *kmer_fwd = seq + i;
        const char *kmer_rev = seqRev + length - i - kmerSize;
        const char * kmer = (noncanonical || memcmp(kmer_fwd, kmer_rev, kmerSize) <= 0) ? kmer_fwd : kmer_rev;
        bool filter = false;
        
        hash_u hash = getHash(kmer, kmerSize, parameters.seed, parameters.use64);
        
		minHashHeap.tryInsert(hash);
    }
    
    if ( ! noncanonical )
    {
        delete [] seqRev;
    }
}

/*void getMinHashPositions(vector<Sketch::PositionHash> & positionHashes, char * seq, uint32_t length, const Sketch::Parameters & parameters, int verbosity)
{
    // Find positions whose hashes are min-hashes in any window of a sequence
    
    int kmerSize = parameters.kmerSize;
    int mins = parameters.minHashesPerWindow;
    int windowSize = parameters.windowSize;
    
    int nextValidKmer = 0;
    
    if ( windowSize > length - kmerSize + 1 )
    {
        windowSize = length - kmerSize + 1;
    }
    
    if ( verbosity > 1 ) cout << seq << endl << endl;
    
    // Associate positions with flags so they can be marked as min-hashes
    // at any point while the window is moved across them
    //
    struct CandidateLocus
    {
        CandidateLocus(int positionNew)
            :
            position(positionNew),
            isMinmer(false)
            {}
        
        int position;
        bool isMinmer;
    };
    
    // All potential min-hash loci in the current window organized by their
    // hashes so repeats can be grouped and so the sorted keys can be used to
    // keep track of the current h bottom hashes. A deque is used here (rather
    // than a standard queue) for each list of candidate loci for the sake of
    // debug output; the performance difference seems to be negligible.
    //
    map<Sketch::hash_t, deque<CandidateLocus>> candidatesByHash;
    
    // Keep references to the candidate loci in the map in the order of the
    // current window, allowing them to be popped off in the correct order as
    // the window is incremented.
    //
    queue<map<Sketch::hash_t, deque<CandidateLocus>>::iterator> windowQueue;
    
    // Keep a reference to the "hth" min-hash to determine efficiently whether
    // new hashes are min-hashes. It must be decremented when a hash is inserted
    // before it.
    //
    map<Sketch::hash_t, deque<CandidateLocus>>::iterator maxMinmer = candidatesByHash.end();
    
    // A reference to the list of potential candidates that has just been pushed
    // onto the back of the rolling window. During the loop, it will be assigned
    // to either an existing list (if the kmer is repeated), a new list, or a
    // dummy iterator (for invalid kmers).
    //
    map<Sketch::hash_t, deque<CandidateLocus>>::iterator newCandidates;
    
    int unique = 0;
    
    for ( int i = 0; i < length - kmerSize + 1; i++ )
    {
        // Increment the next valid kmer if needed. Invalid kmers must still be
        // processed to keep the queue filled, but will be associated with a
        // dummy iterator. (Currently disabled to allow all kmers; see below)
        //
        if ( i >= nextValidKmer )
        {
            for ( int j = i; j < i + kmerSize; j++ )
            {
                char c = seq[j];
                
                if ( c != 'A' && c != 'C' && c != 'G' && c != 'T' )
                {
                    // Uncomment to skip invalid kmers
                    //
                    //nextValidKmer = j + 1;
                    
                    break;
                }
            }
        }
        
        if ( i < nextValidKmer && verbosity > 1 )
        {
            cout << "  [";
        
            for ( int j = i; j < i + kmerSize; j++ )
            {
                cout << seq[j];
            }
            
            cout << "]" << endl;
        }
        
        if ( i >= nextValidKmer )
        {
            Sketch::hash_t hash = getHash(seq + i, kmerSize, parameters.seed, parameters.use64).hash64; // TODO: dynamic
            
            if ( verbosity > 1 )
            {
                cout << "   ";
            
                for ( int j = i; j < i + kmerSize; j++ )
                {
                    cout << seq[j]; 
                }
            
                cout << "   " << i << '\t' << hash << endl;
            }
            
            // Get the list of candidate loci for the current hash (if it is a
            // repeat) or insert a new list.
            //
            pair<map<Sketch::hash_t, deque<CandidateLocus>>::iterator, bool> inserted =
                candidatesByHash.insert(pair<Sketch::hash_t, deque<CandidateLocus>>(hash, deque<CandidateLocus>()));
            newCandidates = inserted.first;
            
            // Add the new candidate locus to the list
            //
            newCandidates->second.push_back(CandidateLocus(i));
            
            if
            (
                inserted.second && // inserted; decrement maxMinmer if...
                (
                    (
                        // ...just reached number of mins
                        
                        maxMinmer == candidatesByHash.end() &&
                        candidatesByHash.size() == mins
                    ) ||
                    (
                        // ...inserted before maxMinmer
                        
                        maxMinmer != candidatesByHash.end() &&
                        newCandidates->first < maxMinmer->first
                    )
                )
            )
            {
                maxMinmer--;
                
                if ( i >= windowSize )
                {
                    unique++;
                }
            }
        }
        else
        {
            // Invalid kmer; use a dummy iterator to pad the queue
            
            newCandidates = candidatesByHash.end();
        }
        
        // Push the new reference to the list of candidate loci for the new kmer
        // on the back of the window to roll it.
        //
        windowQueue.push(newCandidates);
        
        // A reference to the front of the window, to be popped off if the
        // window has grown to full size. This reference can be a dummy if the
        // window is not full size or if the front of the window is a dummy
        // representing an invalid kmer.
        //
        map<Sketch::hash_t, deque<CandidateLocus>>::iterator windowFront = candidatesByHash.end();
        
        if ( windowQueue.size() > windowSize )
        {
            windowFront = windowQueue.front();
            windowQueue.pop();
            
            if ( verbosity > 1 ) cout << "   \tPOP: " << windowFront->first << endl;
        }
        
        if ( windowFront != candidatesByHash.end() )
        {
            deque<CandidateLocus> & frontCandidates = windowFront->second;
            
            if ( frontCandidates.front().isMinmer )
            {
                if ( verbosity > 1 ) cout << "   \t   minmer: " << frontCandidates.front().position << '\t' << windowFront->first << endl;
                positionHashes.push_back(Sketch::PositionHash(frontCandidates.front().position, windowFront->first));
            }
            
            if ( frontCandidates.size() > 1 )
            {
                frontCandidates.pop_front();
                
                // Since this is a repeated hash, only the locus in the front of
                // the list was considered min-hash loci. Check if the new front
                // will become a min-hash so it can be flagged.
                //
                if ( maxMinmer == candidatesByHash.end() || ( i >= windowSize && windowFront->first <= maxMinmer->first) )
                {
                    frontCandidates.front().isMinmer = true;
                }
            }
            else
            {
                // The list for this hash is no longer needed; destroy it,
                // repositioning the reference to the hth min-hash if
                // necessary.
                
                if ( maxMinmer != candidatesByHash.end() && windowFront->first <= maxMinmer->first )
                {
                    maxMinmer++;
                    
                    if ( maxMinmer != candidatesByHash.end() )
                    {
                        maxMinmer->second.front().isMinmer = true;
                    }
                    
                    unique++;
                }
            
                candidatesByHash.erase(windowFront);
            }
        }
        
        if ( i == windowSize - 1 )
        {
            // first complete window; mark min-hashes
            
            for ( map<Sketch::hash_t, deque<CandidateLocus>>::iterator j = candidatesByHash.begin(); j != maxMinmer; j++ )
            {
                j->second.front().isMinmer = true;
            }
            
            if ( maxMinmer != candidatesByHash.end() )
            {
                maxMinmer->second.front().isMinmer = true;
            }
            
            unique++;
        }
        
        // Mark the candidate that was pushed on the back of the window queue
        // earlier as a min-hash if necessary
        //
        if ( newCandidates != candidatesByHash.end() && i >= windowSize && (maxMinmer == candidatesByHash.end() || newCandidates->first <= maxMinmer->first) )
        {
            newCandidates->second.front().isMinmer = true;
        }
        
        if ( verbosity > 1 )
        {
            for ( map<Sketch::hash_t, deque<CandidateLocus>>::iterator j = candidatesByHash.begin(); j != candidatesByHash.end(); j++ )
            {
                cout << "   \t" << j->first;
                
                if ( j == maxMinmer )
                {
                     cout << "*";
                }
                
                for ( deque<CandidateLocus>::iterator k = j->second.begin(); k != j->second.end(); k++ )
                {
                    cout << '\t' << k->position;
                    
                    if ( k->isMinmer )
                    {
                        cout << '!';
                    }
                }
                
                cout << endl;
            }
        }
    }
    
    // finalize remaining min-hashes from the last window
    //
    while ( windowQueue.size() > 0 )
    {
        map<Sketch::hash_t, deque<CandidateLocus>>::iterator windowFront = windowQueue.front();
        windowQueue.pop();
        
        if ( windowFront != candidatesByHash.end() )
        {
            deque<CandidateLocus> & frontCandidates = windowFront->second;
            
            if ( frontCandidates.size() > 0 )
            {
                if ( frontCandidates.front().isMinmer )
                {
                    if ( verbosity > 1 ) cout << "   \t   minmer:" << frontCandidates.front().position << '\t' << windowFront->first << endl;
                    positionHashes.push_back(Sketch::PositionHash(frontCandidates.front().position, windowFront->first));
                }
                
                frontCandidates.pop_front();
            }
        }
    }
    
    if ( verbosity > 1 )
    {
        cout << endl << "Minmers:" << endl;
    
        for ( int i = 0; i < positionHashes.size(); i++ )
        {
            cout << "   " << positionHashes.at(i).position << '\t' << positionHashes.at(i).hash << endl;
        }
        
        cout << endl;
    }
    
    if ( verbosity > 0 ) cout << "   " << positionHashes.size() << " minmers across " << length - windowSize - kmerSize + 2 << " windows (" << unique << " windows with distinct minmer sets)." << endl << endl;
}*/



bool hasSuffixFingerPrint(string const & whole, string const & suffix)
{
    if (whole.length() >= suffix.length())
    {
        return 0 == whole.compare(whole.length() - suffix.length(), suffix.length(), suffix);
    }
    
    return false;
}

/*Sketch::SketchOutput * loadCapnp(Sketch::SketchInput * input)
{
	const char * file = input->fileNames[0].c_str();
    int fd = open(file, O_RDONLY);
    
    struct stat fileInfo;
    
    if ( stat(file, &fileInfo) == -1 )
    {
        return 0;
    }
    
	Sketch::SketchOutput * output = new Sketch::SketchOutput();
	vector<Sketch::Reference> & references = output->references;
	
    void * data = mmap(NULL, fileInfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    capnp::ReaderOptions readerOptions;
    
    readerOptions.traversalLimitInWords = 1000000000000;
    readerOptions.nestingLimit = 1000000;
    
    capnp::FlatArrayMessageReader * message = new capnp::FlatArrayMessageReader(kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word *>(data), fileInfo.st_size / sizeof(capnp::word)), readerOptions);
    capnp::MinHash::Reader reader = message->getRoot<capnp::MinHash>();
    
    capnp::MinHash::ReferenceList::Reader referenceListReader = reader.getReferenceList().getReferences().size() ? reader.getReferenceList() : reader.getReferenceListOld();
    
    capnp::List<capnp::MinHash::ReferenceList::Reference>::Reader referencesReader = referenceListReader.getReferences();
    
    references.resize(referencesReader.size());
    
    for ( uint64_t i = 0; i < referencesReader.size(); i++ )
    {
        capnp::MinHash::ReferenceList::Reference::Reader referenceReader = referencesReader[i];
        
        Sketch::Reference & reference = references[i];
        reference.id = referenceReader.getId();
        reference.name = referenceReader.getName();
        reference.comment = referenceReader.getComment();
        
        if ( referenceReader.getLength64() )
        {
        	reference.length = referenceReader.getLength64();
        }
        else
        {
	        reference.length = referenceReader.getLength();
	    }
        
        reference.hashesSorted.setUse64(input->parameters.use64);
        uint64_t hashCount;
        
        if ( input->parameters.use64 )
        {
            capnp::List<uint64_t>::Reader hashesReader = referenceReader.getHashes64();
        
        	hashCount = hashesReader.size();
        	
        	if ( hashCount > input->parameters.minHashesPerWindow )
        	{
        		hashCount = input->parameters.minHashesPerWindow;
        	}
        	
            reference.hashesSorted.resize(hashCount);
        
            for ( uint64_t j = 0; j < hashCount; j++ )
            {
                reference.hashesSorted.set64(j, hashesReader[j]);
            }
        }
        else
        {
            capnp::List<uint32_t>::Reader hashesReader = referenceReader.getHashes32();
        	
        	hashCount = hashesReader.size();
        	
        	if ( hashCount > input->parameters.minHashesPerWindow )
        	{
        		hashCount = input->parameters.minHashesPerWindow;
        	}
        	
            reference.hashesSorted.resize(hashCount);
        
            for ( uint64_t j = 0; j < hashCount; j++ )
            {
                reference.hashesSorted.set32(j, hashesReader[j]);
            }
        }
        
        if ( referenceReader.hasCounts32() )
        {
			capnp::List<uint32_t>::Reader countsReader = referenceReader.getCounts32();
		
			reference.counts.resize(hashCount);
		
			for ( uint64_t j = 0; j < hashCount; j++ )
			{
				reference.counts[j] = countsReader[j];
			}
        }
        
        reference.countsSorted = referenceReader.getCounts32Sorted();
    }
    
    //capnp::MinHash::LocusList::Reader locusListReader = reader.getLocusList();
    //capnp::List<capnp::MinHash::LocusList::Locus>::Reader lociReader = locusListReader.getLoci();
    
    output->positionHashesByReference.resize(references.size());
    
    for ( uint64_t i = 0; i < lociReader.size(); i++ )
    {
        //capnp::MinHash::LocusList::Locus::Reader locusReader = lociReader[i];
        //cout << locusReader.getHash64() << '\t' << locusReader.getSequence() << '\t' << locusReader.getPosition() << endl;
        //output->positionHashesByReference[locusReader.getSequence()].push_back(Sketch::PositionHash(locusReader.getPosition(), locusReader.getHash64()));
    }
    
    cout << endl << "References:" << endl << endl;
    
    vector< vector<string> > columns(3);
    
    columns[0].push_back("ID");
    columns[1].push_back("Length");
    columns[2].push_back("Name/Comment");
    
    for ( uint64_t i = 0; i < references.size(); i++ )
    {
        columns[0].push_back(to_string(i));
        columns[1].push_back(to_string(references[i].length));
        columns[2].push_back(references[i].name + " " + references[i].comment);
    }
    
    printColumns(columns);
    cout << endl;
    
    
    printf("\nCombined hash table:\n");
    
    cout << "   kmer:  " << kmerSize << endl;
    cout << "   comp:  " << compressionFactor << endl << endl;
    
    for ( LociByHash_umap::iterator i = lociByHash.begin(); i != lociByHash.end(); i++ )
    {
        printf("Hash %u:\n", i->first);
        
        for ( int j = 0; j < i->second.size(); j++ )
        {
            printf("   Seq: %d\tPos: %d\n", i->second.at(j).sequence, i->second.at(j).position);
        }
    }
    
    cout << endl;
    
    munmap(data, fileInfo.st_size);
    close(fd);
    delete message;
    
    return output;
}*/


SketchFingerPrint::SketchOutput * loadCapnpFingerPrint(SketchFingerPrint::SketchInput * input)
{
	const char * file = input->fileNames[0].c_str();
    int fd = open(file, O_RDONLY);
    
    struct stat fileInfo;
    
    if ( stat(file, &fileInfo) == -1 )
    {
        return 0;
    }
    
	SketchFingerPrint::SketchOutput * output = new SketchFingerPrint::SketchOutput();
	vector<SketchFingerPrint::Reference> & references = output->references;
	
    void * data = mmap(NULL, fileInfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    capnp::ReaderOptions readerOptions;
    
    readerOptions.traversalLimitInWords = 1000000000000;
    readerOptions.nestingLimit = 1000000;
    
    capnp::FlatArrayMessageReader * message = new capnp::FlatArrayMessageReader(kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word *>(data), fileInfo.st_size / sizeof(capnp::word)), readerOptions);
    capnp::MinHashFingerPrint::Reader reader = message->getRoot<capnp::MinHashFingerPrint>();
    
    capnp::MinHashFingerPrint::ReferenceList::Reader referenceListReader = reader.getReferenceList().getReferences().size() ? reader.getReferenceList() : reader.getReferenceListOld();

    
    capnp::List<capnp::MinHashFingerPrint::ReferenceList::Reference>::Reader referencesReader = referenceListReader.getReferences();
    
    references.resize(referencesReader.size());

    for ( uint64_t i = 0; i < referencesReader.size(); i++ )
    {
        capnp::MinHashFingerPrint::ReferenceList::Reference::Reader referenceReader = referencesReader[i];
        
        SketchFingerPrint::Reference & reference = references[i];

        reference.id = referenceReader.getId();
        reference.name = referenceReader.getName();
        reference.comment = referenceReader.getComment();
        reference.use64 = referenceReader.getUseHash64();


        if ( referenceReader.getLength64() )
        {
        	reference.length = referenceReader.getLength64();
        }
        else
        {
	        reference.length = referenceReader.getLength();
	    }
        
        uint64_t hashCount;

        // Verifico che i miei subsketch hanno una size >0 
        if(referenceReader.getSubSketchList().size()!=0){

            // Devo effettuare la resize dello sketch 
            reference.subSketch_list.resize(referenceReader.getSubSketchList().size());


            // Devo prendere la lista dei subsketch 
            capnp::List<capnp::MinHashFingerPrint::HashList>::Reader subSketchHashListReader = referenceReader.getSubSketchList();
            for(uint64_t j = 0; j < subSketchHashListReader.size(); j++){
                capnp::MinHashFingerPrint::HashList::Reader hashListReader = subSketchHashListReader[j];

                // Crea una nuova istanza di HashList
                HashList hashList;

                if ( true ){

                    capnp::List<uint64_t>::Reader hashesReader = hashListReader.getHashList64();

                
                    hashCount = hashListReader.getHashList64().size();

                    
                    if ( hashCount > input->parameters.minHashesPerWindow )
                    {
                        hashCount = input->parameters.minHashesPerWindow;
                    }
                    
                
                    for ( uint64_t k = 0; k < hashCount; k++ )
                    {
                        hashList.push_back64(hashesReader[k]);
                    }
                    
                }
                else{
                    
                    capnp::List<uint32_t>::Reader hashesReader = hashListReader.getHashList32();

                
                    hashCount = hashListReader.getHashList32().size();

                    
                    if ( hashCount > input->parameters.minHashesPerWindow )
                    {
                        hashCount = input->parameters.minHashesPerWindow;
                    }
                    
                
                    for ( uint32_t k = 0; k < hashCount; k++ )
                    {
                        hashList.push_back32(hashesReader[k]);
                    }

                }

                reference.subSketch_list.at(j) = hashList;
            }
        }
    }
    
    munmap(data, fileInfo.st_size);
    close(fd);
    delete message;
    
    return output;
}




/* Array from 0..25 of DNA complement of A..Z */
const char complement[] = {
  'T', // 'A' = A
  'V', // 'B' = not A = C,T,G
  'G', // 'C' = C
  'H', // 'D' = not C = A,T,G
  'N', // 'E' = .
  'N', // 'F' = .
  'C', // 'G' = G
  'D', // 'H' = not G = A,C,T
  'N', // 'I' = .
  'N', // 'J' = .
  'M', // 'K' = T,G = Keto
  'N', // 'L' = .
  'K', // 'M' = A,C = Amino
  'N', // 'N' = A,C,T,G = uNkNowN
  'N', // 'O' = .
  'N', // 'P' = .
  'N', // 'Q' = .
  'Y', // 'R' = A,G = puRine
  'S', // 'S' = G,C = Strong
  'A', // 'T' = T
  'A', // 'U' = T (RNA)
  'B', // 'V' = not T = A,C,G
  'W', // 'W' = A,T = Weak
  'N', // 'X' = .
  'R', // 'Y' = pYrimidine = C,T
  'N', // 'Z' = .
};

void reverseFingerPrintComplement(const char * src, char * dest, int length)
{
    for ( int i = 0; i < length; i++ )
    {
        dest[i] = complement[ (int) src[length-i-1] - (int) 'A' ];
    }
}

void setAlphabetFingerPrintFromString(SketchFingerPrint::Parameters & parameters, const char * characters)
{
    parameters.alphabetSize = 0;
    memset(parameters.alphabet, 0, 256);
    
	const char * character = characters;
	
	while ( *character != 0 )
	{
		char characterUpper = *character;
		
        if ( ! parameters.preserveCase && characterUpper > 96 && characterUpper < 123 )
        {
            characterUpper -= 32;
        }
        
		parameters.alphabet[characterUpper] = true;
		character++;
	}
    
    for ( int i = 0; i < 256; i++ )
    {
    	if ( parameters.alphabet[i] )
    	{
	    	parameters.alphabetSize++;
	    }
    }
    
    parameters.use64 = pow(parameters.alphabetSize, parameters.kmerSize) > pow(2, 32);
}

void setMinHashesForReference(SketchFingerPrint::Reference & reference, const MinHashHeap & hashes)
{
    /*HashList & hashList = reference.hashesSorted;
    hashList.clear();
    hashes.toHashList(hashList, reference.counts);
    reference.countsSorted = true;*/
}

SketchFingerPrint::SketchOutput * sketchFile(SketchFingerPrint::SketchInput * input)
{
	const SketchFingerPrint::Parameters & parameters = input->parameters;
	
	SketchFingerPrint::SketchOutput * output = new SketchFingerPrint::SketchOutput();
	
	output->references.resize(1);
	SketchFingerPrint::Reference & reference = output->references[0];
	
    MinHashHeap minHashHeap(parameters.use64, parameters.minHashesPerWindow, parameters.reads ? parameters.minCov : 1, parameters.memoryBound);

	reference.length = 0;
	//reference.hashesSorted.setUse64(parameters.use64);
	
    int l;
    int count = 0;
	bool skipped = false;
	
	int fileCount = input->fileNames.size();
	gzFile fps[fileCount];
	list<kseq_t *> kseqs;
	//
	for ( int f = 0; f < fileCount; f++ )
	{
		if ( input->fileNames[f] == "-" )
		{
			if ( f > 1 )
			{
				cerr << "ERROR: '-' for stdin must be first input" << endl;
				exit(1);
			}
			
			fps[f] = gzdopen(fileno(stdin), "r");
		}
		else
		{
			if ( reference.name == "" && input->fileNames[f] != "-" )
			{
				reference.name = input->fileNames[f];
			}
			
			fps[f] = gzopen(input->fileNames[f].c_str(), "r");
			
			if ( fps[f] == 0 )
			{
				cerr << "ERROR: could not open " << input->fileNames[f] << endl;
				exit(1);
			}
		}
		
		kseqs.push_back(kseq_init(fps[f]));
	}
	
	list<kseq_t *>::iterator it = kseqs.begin();
	
	while ( kseqs.begin() != kseqs.end() )
	{
		l = kseq_read(*it);
		
		if ( l < -1 ) // error
		{
			break;
		}
		
		if ( l == -1 ) // eof
		{
			kseq_destroy(*it);
			it = kseqs.erase(it);
			if ( it == kseqs.end() )
			{
				it = kseqs.begin();
			}
			continue;
		}
		
		if ( l < parameters.kmerSize )
		{
			skipped = true;
			continue;
		}
		
		if ( count == 0 )
		{
			if ( input->fileNames[0] == "-" )
			{
				reference.name = (*it)->name.s;
				reference.comment = (*it)->comment.s ? (*it)->comment.s : "";
			}
			else
			{
				reference.comment = (*it)->name.s;
				reference.comment.append(" ");
				reference.comment.append((*it)->comment.s ? (*it)->comment.s : "");
			}
		}
		
		count++;
		
		
		//if ( verbosity > 0 && parameters.windowed ) cout << '>' << seq->name.s << " (" << l << "nt)" << endl << endl;
		//if (seq->comment.l) printf("comment: %s\n", seq->comment.s);
		//printf("seq: %s\n", seq->seq.s);
		//if (seq->qual.l) printf("qual: %s\n", seq->qual.s);
		
		if ( ! parameters.reads )
		{
			reference.length += l;
		}
		
		addMinHashes(minHashHeap, (*it)->seq.s, l, parameters);
		
		if ( parameters.reads && parameters.targetCov > 0 && minHashHeap.estimateMultiplicity() >= parameters.targetCov )
		{
			l = -1; // success code
			break;
		}
		
		it++;
		
		if ( it == kseqs.end() )
		{
			it = kseqs.begin();
		}
	}
	
	if ( parameters.reads )
	{
		if ( parameters.genomeSize != 0 )
		{
			reference.length = parameters.genomeSize;
		}
		else
		{
			reference.length = minHashHeap.estimateSetSize();
		}
	}
	
	if ( count > 1 )
	{
		reference.comment.insert(0, " seqs] ");
		reference.comment.insert(0, to_string(count));
		reference.comment.insert(0, "[");
		reference.comment.append(" [...]");
		//reference.comment.append(to_string(count - 1));
		//reference.comment.append(" more]");
	}
	
	if (  l != -1 )
	{
		cerr << "\nERROR: reading " << (input->fileNames.size() > 0 ? "input files" : input->fileNames[0]) << "." << endl;
		exit(1);
	}
	
	if ( reference.length == 0 )
	{
		if ( skipped )
		{
			cerr << "\nWARNING: All fasta records in " << (input->fileNames.size() > 0 ? "input files" : input->fileNames[0]) << " were shorter than the k-mer size (" << parameters.kmerSize << ")." << endl;
		}
		else
		{
			cerr << "\nERROR: Did not find fasta records in \"" << (input->fileNames.size() > 0 ? "input files" : input->fileNames[0]) << "\"." << endl;
		}
		
		exit(1);
	}
	
	if ( ! parameters.windowed )
	{
		setMinHashesForReference(reference, minHashHeap);
	}
	
    if ( parameters.reads )
    {
       	cerr << "Estimated genome size: " << minHashHeap.estimateSetSize() << endl;
    	cerr << "Estimated coverage:    " << minHashHeap.estimateMultiplicity() << endl;
    	
    	if ( parameters.targetCov > 0 )
    	{
	    	cerr << "Reads used:            " << count << endl;
	    }
    }
	
	for ( int i = 0; i < fileCount; i++ )
	{
		gzclose(fps[i]);
	}
	
	return output;
}

SketchFingerPrint::SketchOutput * sketchSequence(SketchFingerPrint::SketchInput * input)
{
	const SketchFingerPrint::Parameters & parametersFingerPrint = input->parameters;
	
	SketchFingerPrint::SketchOutput * output = new SketchFingerPrint::SketchOutput();
	
	output->references.resize(1);
	SketchFingerPrint::Reference & reference = output->references[0];
	
	reference.length = input->length;
	reference.name = input->name;
	reference.comment = input->comment;
	reference.use64 = parametersFingerPrint.use64 ;
	
	if ( parametersFingerPrint.windowed )
	{
		//output->positionHashesByReference.resize(1);
		//getMinHashPositions(output->positionHashesByReference[0], input->seq, input->length, parameters, 0);
	}
	else
	{
	    MinHashHeap minHashHeap(parametersFingerPrint.use64, parametersFingerPrint.minHashesPerWindow, parametersFingerPrint.reads ? parametersFingerPrint.minCov : 1);
        addMinHashes(minHashHeap, input->seq, input->length, parametersFingerPrint);
		setMinHashesForReference(reference, minHashHeap);
	}
	
	return output;
}








// The following functions are adapted from http://www.zlib.net/zpipe.c


/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int defFingerPrint(int fdSource, int fdDest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = read(fdSource, in, CHUNK);
        if (strm.avail_in == -1) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = strm.avail_in == 0 ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (write(fdDest, out, have) != have) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int infFingerPrint(int fdSource, int fdDest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = read(fdSource, in, CHUNK);
        if (strm.avail_in == -1) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (write(fdDest, out, have) != have) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerrFingerPrint(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}