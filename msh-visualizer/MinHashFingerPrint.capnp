@0xd1432ff53bcedbc4;

struct MinHashFingerPrint
{
    struct ReferenceList
    {
        struct Reference
        {
            id @0: Text;
            sequence @1 : Text;
            quality @2 : Text;
            length @3 : UInt32;
            name @4 : Text;
            comment @5 : Text;
            length64 @6 : UInt64;

            counts32 @7 : List(UInt32);
            counts32Sorted @8 : Bool;

            subSketchList @9 : List(HashList);

            useHash64 @10 : Bool;

        }


        
        references @0 : List(Reference);
    }


    
    struct HashList{

        hashList64 @0 : List(UInt64);
        hashList32 @1 : List(UInt32);
    }

    
    
    kmerSize @0 : UInt32;
    windowSize @1 : UInt32;
    minHashesPerWindow @2 : UInt32;
    concatenated @3 : Bool;
    error @5 : Float32;
    noncanonical @6 : Bool;
    alphabet @7 : Text;
    preserveCase @8 : Bool;
    hashSeed @9 : UInt32 = 42;
    
    referenceListOld @4 : ReferenceList;
    referenceList @10 : ReferenceList;
}