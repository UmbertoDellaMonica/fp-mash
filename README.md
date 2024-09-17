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

### 3 - Generate sketch files ( .msh )

Use this command to generate the sketch files

```bash
   mash sketch -fp ../training/Umberto/CFL/DNA3-CFL.txt  -o ../training/Umberto/CFL/DNA3-sketch.msh
```

##### Mash'parameters - Command Sketch

- **skech** : it is a command of mash to generate a sketch file with format .msh
- **-fp** : this parameter means the type of operation that we want apply (fingerprint way)
- **-o** : this parameter means the name and path where the file is saved  

### 4 - Generate Info sketch files (.json)

If we want see files in format .msh , we need to run this command :

```bash
   mash info -d ../training/Umberto/CFL/DNA3-CFL.txt  > ../training/Umberto/CFL/DNA3-sketch.json
```

##### Mash'parameters - Command Info

- **info** : It is a command of mash that is used to view  all info of a sketch file  
- **-d** : this parameter means "dump". We want store or view the information in a clear view like file .json
- **>** : this is a command of shell Unix like, because we want redirect the standard output in out file .json  

### 5 - Distance between 2 FingerPrint sketch files

Sometimes we need to view the distance between a dna sequence and the command **distance** is for us

In this case, there is an example of usage the method of dist between two sketch files

```bash
   mash dist -fp  ../training/Umberto/CFL/DNA3-sketch.msh ../training/Umberto/CFL/DNA2-sketch.msh 
```

##### Mash'parameters - Command Info

- **dist** : It is a command of mash that is used to see the distance between two sketch files.
The **distance** is a value between 0-1. if the value is near to 0 in this case we have a similarity between two sketch files instead we have two different files if we have a value near to 1

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
