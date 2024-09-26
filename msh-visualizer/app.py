import capnp
import MinHashFingerPrint_capnp  # Assicurati che il nome del file sia corretto

def print_sketch(sketch):
    print("=== MinHash Parameters ===")
    print(f"Kmer Size: {sketch.kmerSize}")
    print(f"Window Size: {sketch.windowSize}")
    print(f"Min Hashes Per Window: {sketch.minHashesPerWindow}")
    print(f"Concatenated: {sketch.concatenated}")
    print(f"Error: {sketch.error}")
    print(f"Noncanonical: {sketch.noncanonical}")
    print(f"Alphabet: {sketch.alphabet}")
    print(f"Preserve Case: {sketch.preserveCase}")
    print(f"Hash Seed: {sketch.hashSeed}")
    print()

    print("=== Reference List ===")
    list_reference_old = sketch.referenceListOld
    print(f"List of Old Reference: {list_reference_old}")

    for ref in sketch.referenceList.references:
        print(f"Reference ID: {ref.id}")
        print(f"Sequence: {ref.sequence}")
        print(f"Quality: {ref.quality}")
        print(f"Length: {ref.length}")
        print(f"Length64: {ref.length64}")
        print(f"Name: {ref.name}")
        print(f"Comment: {ref.comment}")
        print(f"Counts32 Sorted: {ref.counts32Sorted}")

        if ref.counts32:
            print("Counts32:")
            for count in ref.counts32:
                print(f"  {count}")

        print("SubSketches:")
        for subSketch in ref.subSketchList:
            print(f"  SubSketch:")
            if subSketch.hashList32:
                print("  Hashes32:")
                for hash32 in subSketch.hashList32:
                    print(f"    {hash32}")
            if subSketch.hashList64:
                print("  Hashes64:")
                for hash64 in subSketch.hashList64:
                    print(f"    {hash64}")

        print()

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <file.msh>")
        sys.exit(1)

    filename = sys.argv[1]
    with open(filename, 'rb') as f:
        sketch = MinHashFingerPrint_capnp.MinHashFingerPrint.read(f)
        print_sketch(sketch)
