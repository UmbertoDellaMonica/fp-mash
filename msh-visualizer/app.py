import capnp
import MinHash_capnp  # Assicurati che il nome del file sia corretto

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
    print(f"List of Reference : {list_reference_old}")


    for ref in sketch.referenceList.references:
        print(f"Reference ID: {ref.id}")
        print(f"Sequence: {ref.sequence}")
        print(f"Quality: {ref.quality}")
        print(f"Length: {ref.length}")
        print(f"Length64: {ref.length64}")
        print(f"Name: {ref.name}")
        print(f"Comment: {ref.comment}")
        print(f"Counts32 Sorted: {ref.counts32Sorted}")

        if ref.hashes32:
            print("Hashes32:")
            for hash32 in ref.hashes32:
                print(f"  {hash32}")

        if ref.hashes64:
            print("Hashes64:")
            for hash64 in ref.hashes64:
                print(f"  {hash64}")

        print("SubSketches:")
        for subSketch in ref.subSketchList:
            print(f"  SubSketch ID: {subSketch.id}")
            if subSketch.hashes32:
                print("  Hashes32:")
                for hash32 in subSketch.hashes32:
                    print(f"    {hash32}")
            if subSketch.hashes64:
                print("  Hashes64:")
                for hash64 in subSketch.hashes64:
                    print(f"    {hash64}")

        print()

    print("=== Locus List ===")
    for locus in sketch.locusList.loci:
        print(f"Locus Sequence: {locus.sequence}, Position: {locus.position}, Hash32: {locus.hash32}, Hash64: {locus.hash64}")

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <file.msh>")
        sys.exit(1)

    filename = sys.argv[1]
    with open(filename, 'rb') as f:
        sketch = MinHash_capnp.MinHash.read(f)
        print_sketch(sketch)
