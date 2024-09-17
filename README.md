# fp-mash

## Overview

This project, part of the Bioinformatics course for the Master's degree in Computer Science at the University of Salerno, aims to leverage bioinformatics tools and techniques to analyze and interpret biological data. The main focus is to adapt the features of **Mash** for the applications of **lyn2vec**.

## Tools and Technologies

- **Mash**: A tool for fast genome and metagenome distance estimation using MinHash.
  - [GitHub Repository](https://github.com/marbl/Mash)
- **lyn2vec**: A project for embedding sequences into vector space for downstream machine learning applications.
  - [GitHub Repository](https://github.com/rzaccagnino/lyn2vec)

# Installation

- **Mash** : Follow the **INSTALL.TXT** file for a better comprehension
- **Lyn2vec**: Clone the repository and install the requirements  

## Features

- Fast genome and metagenome distance estimation
- Sequence vectorization for machine learning
- Integration of multiple bioinformatics tools for comprehensive analysis
- Factorization of DNA sequence in k-fingers
- Reuse Mash features for Lyn2vec factorization

## Usage

### 1 - Generate DNA sequence

Generate pseudo DNA sequence with this command

```bash
    python lyn2vec.py --type generate --path ../training/Umberto/CFL/DNA3 --size 2000 --gc_content 0.6 --format fasta --number_dna_generate 5
```

##### Parameters

- **--type** : usage of parameter to indicate the type of action on Lyn2vec
- **--size** : this parameters is the length for each DNA sequence
- **--number_dna_generate** : is the number of DNA that we want generate in a file  
- **--format**: this is the format of a file. It is valid also ( .fa , .fasta , .fastq )
- **--gc_content** : this value is set between to **[0-1]**  

### 2 - Generate K-FingerPrints

#### K-fingerprints

Use this command to generate the **fingerprint factorization**

```bash
   python lyn2vec.py --type basic --path ../training/Umberto/CFL/ --fasta DNA3.fasta --type_factorization CFL   --rev_com true -n 4
```

##### Parameters

- **--type** : usage of parameter to indicate the type of action on Lyn2vec **[ basic , generalized , mapping , generate ]**
- **--size** : this parameters is the length for each DNA sequence
- **--number_dna_generate** : is the number of DNA that we want generate in a file  
- **--fasta**: Indicate the file (.fasta, .fa, .fastq, .gz ) that we have generated before
- **--type_factorization** : that parameter indicates the type of factorization that we want to use to generate the fingerPrint. We have this type of factorization **[CFL, ICFL , ICFL_COMB, CFL_ICFL,CFL_ICFL_COMB]**

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## License Information for Dependencies

### Mash

- **KSeq**: MIT License
- **MurmurHash3**: Public Domain
- **Open Bloom Filter**: Common Public License
- **Robin_Hood Unordered Map and Set**: MIT License

### lyn2vec

- No specific license information available.
