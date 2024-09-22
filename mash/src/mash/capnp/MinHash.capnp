using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnp");

@0xc4c8b1ada05e7704;

struct MinHash
{
    struct ReferenceList
    {
        struct Reference
        {
            sequence @0 : Text;
            quality @1 : Text;
            length @2 : UInt32;
            length64 @7 : UInt64;
            name @3 : Text;
            comment @4 : Text;
            subSketchList @10 : List(SubSketch);
            hashes32 @5 : List(UInt32);
            hashes64 @6 : List(UInt64);
            counts32 @8 : List(UInt32);
            counts32Sorted @9 : Bool;
        }
        
        references @0 : List(Reference);
    }


	struct SubSketch{
		id @0 : Text;
		hashes32 @1 : List(UInt32);
		hashes64 @2 : List(UInt64);
    }

    
    struct LocusList
    {
        struct Locus
        {
            sequence @0 : UInt32;
            position @1 : UInt32;
            hash32 @2 : UInt32;
            hash64 @3 : UInt64;
        }
        
        loci @0 : List(Locus);
    }
    
    kmerSize @0 : UInt32;
    windowSize @1 : UInt32;
    minHashesPerWindow @2 : UInt32;
    concatenated @3 : Bool;
    error @6 : Float32;
    noncanonical @7 : Bool;
    alphabet @8 : Text;
    preserveCase @9 : Bool;
    hashSeed @10 : UInt32 = 42;
    
    referenceListOld @4 : ReferenceList;
    referenceList @11 : ReferenceList;
    locusList @5 : LocusList;
}
