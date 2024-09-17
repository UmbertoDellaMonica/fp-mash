import argparse

from fingerprint_utils import *
from multiprocessing.pool import Pool
from functools import partial

from factorizations import CFL, ICFL_recursive, CFL_icfl
from factorizations_comb import d_cfl, d_icfl, d_cfl_icfl

from dna_utils import generate_dna_sequences,generate_transcript_short_id,generate_genes_short_id

########################################################################################################################
# Create fingerprint files (args.step = 'basic') #################################################################
def basic_fingerprint(args):

    # Input FASTA file containing transcripts
    input_fasta = args.path + args.fasta

    # Extract of reads (Format = ID GENE read)
    read_lines = extract_reads(name_file=input_fasta, filter=args.filter, rev_com=args.rev_comb)

    #print_lines(read_lines)

    if len(read_lines) == 0:
        print('No reads extracted!')
        exit(-1)

    print('\nCompute fingerprint by list (%s, %s) - start...' % (args.type_factorization, args.fact))

    fingerprint_file = open("%s" % args.path + "fingerprint_" + args.type_factorization + ".txt", 'w')
    fact_fingerprint_file = None
    if args.fact == 'create':
        # Create file containing factorizations
        fact_fingerprint_file = open("%s" % args.path + "fact_fingerprint_" + args.type_factorization + ".txt", 'w')

    # SPLIT for multiprocessing
    size = int(len(read_lines)/args.n)
    splitted_lines = [read_lines[i:i + size] for i in range(0, len(read_lines), size)]

    with Pool(args.n) as pool:

        type_factorization = args.type_factorization

        # Check type factorization
        factorization = None
        T = None
        if type_factorization == "CFL":
            factorization = CFL
        elif type_factorization == "ICFL":
            factorization = ICFL_recursive
        elif type_factorization == "CFL_ICFL-10":
            factorization = CFL_icfl
            T = 10
        elif type_factorization == "CFL_ICFL-20":
            factorization = CFL_icfl
            T = 20
        elif type_factorization == "CFL_ICFL-30":
            factorization = CFL_icfl
            T = 30
        elif type_factorization == "CFL_COMB":
            factorization = d_cfl
        elif type_factorization == "ICFL_COMB":
            factorization = d_icfl
        elif type_factorization == "CFL_ICFL_COMB-10":
            factorization = d_cfl_icfl
            T = 10
        elif type_factorization == "CFL_ICFL_COMB-20":
            factorization = d_cfl_icfl
            T = 20
        elif type_factorization == "CFL_ICFL_COMB-30":
            factorization = d_cfl_icfl
            T = 30

        func = partial(compute_fingerprint_by_list, args.fact, args.shift, factorization, T)

        fingerprint_lines = []
        fingerprint_fact_lines = []
        for res in pool.map(func, splitted_lines):

            fingerprint_lines = fingerprint_lines + res[0]
            fingerprint_fact_lines = fingerprint_fact_lines + res[1]

        fingerprint_file.writelines(fingerprint_lines)
        if args.fact == 'create':
            fact_fingerprint_file.writelines(fingerprint_fact_lines)

        fingerprint_file.close()

        if args.fact == 'create':
            fact_fingerprint_file.close()

        print('\nCompute fingerprint by list (%s, %s) - stop!' % (args.type_factorization, args.fact))
########################################################################################################################




########################################################################################################################
# Create fingerprint files (args.step = 'generalized') #######################################################################
def generalized_fingerprint(args):

    # Input FASTA file containing transcripts
    input_fasta = args.path + args.fasta

    # Extract of long reads (Format = ID GENE read)
    read_lines = extract_long_reads(name_file=input_fasta,rev_com=args.rev_comb)
    print("read_lines SIZE: ", len(read_lines))
    if len(read_lines) == 0:
        print('No reads extracted!')
        exit(-1)

    print('\nCompute fingerprint by list (%s, %s) - start...' % (args.type_factorization, args.fact))

    fingerprint_file = open("%s" % args.path + "fingerprint_" + args.type_factorization + ".txt", 'w')
    fact_fingerprint_file = None
    if args.fact == 'create':
        # Create file containing factorizations
        fact_fingerprint_file = open("%s" % args.path + "fact_fingerprint_" + args.type_factorization + ".txt", 'w')

    # SPLIT for multiprocessing
    size = int(len(read_lines)/args.n)
    splitted_lines = [read_lines[i:i + size] for i in range(0, len(read_lines), size)]

    with Pool(args.n) as pool:

        type_factorization = args.type_factorization

        # Check type factorization
        factorization = None
        T = None
        if type_factorization == "CFL":
            factorization = CFL
        if type_factorization == "ICFL":
            factorization = ICFL_recursive
        if type_factorization == "CFL_ICFL-10":
            factorization = CFL_icfl
            T = 10
        if type_factorization == "CFL_ICFL-20":
            factorization = CFL_icfl
            T = 20
        if type_factorization == "CFL_ICFL-30":
            factorization = CFL_icfl
            T = 30
        if type_factorization == "CFL_COMB":
            factorization = d_cfl
        if type_factorization == "ICFL_COMB":
            factorization = d_icfl
        if type_factorization == "CFL_ICFL_COMB-10":
            factorization = d_cfl_icfl
            T = 10
        if type_factorization == "CFL_ICFL_COMB-20":
            factorization = d_cfl_icfl
            T = 20
        if type_factorization == 'CFL_ICFL_COMB-30':
            factorization = d_cfl_icfl
            T = 30

        func = partial(compute_long_fingerprint_by_list, args.fact, factorization, T, args.split)

        fingerprint_lines = []
        fingerprint_fact_lines = []
        for res in pool.map(func, splitted_lines):

            fingerprint_lines = fingerprint_lines + res[0]
            fingerprint_fact_lines = fingerprint_fact_lines + res[1]

        fingerprint_file.writelines(fingerprint_lines)
        if args.fact == 'create':
            fact_fingerprint_file.writelines(fingerprint_fact_lines)

        fingerprint_file.close()

        if args.fact == 'create':
            fact_fingerprint_file.close()

        print('\nCompute fingerprint by list (%s, %s) - stop!' % (args.type_factorization, args.fact))
########################################################################################################################



########################################################################################################################
# Create fingerprint files (args.step = 'mapping') #######################################################################
def fingerprint_mapping(args):
    # Input FASTA file containing transcripts
    input_fasta = args.path + args.fingerprint

    mapped_file = open("%s" % args.path + "mapped_" + args.fingerprint + ".txt", 'w')
    mapped_lines = mapping_projection(input_fasta)
    mapped_file.writelines(mapped_lines)
    mapped_file.close()
########################################################################################################################


########################################################################################################################
# Create fasta files (args.step = 'generate') #######################################################################
def generate_fasta_file(sequences, file_name, format='fasta'):
    """
    Genera un file di sequenze in formato .fasta, .fa, o .fastq.

    Args:
        sequences (list of str): Lista delle sequenze di DNA da scrivere.
        file_name (str): Il nome del file da salvare (senza estensione).
        format (str): Il formato del file da generare ('fasta', 'fa', 'fastq').
    
    Raises:
        ValueError: Se il formato fornito non è supportato.
    """
    if format not in ['fasta', 'fa', 'fastq']:
        raise ValueError("Formato non supportato. Utilizzare 'fasta', 'fa' o 'fastq'.")

    with open(f"{file_name}.{format}", 'w+') as file:
        for sequence in sequences:
            ID_TRANSCRIPT_GENE = generate_transcript_short_id()
            ID_GENE = generate_genes_short_id(ID_TRANSCRIPT_GENE)

            header = f">{ID_TRANSCRIPT_GENE} {ID_GENE}\n"
            new_sequence = '\n'.join([sequence[i:i+70] for i in range(0, len(sequence), 70)])

            if format in ['fasta', 'fa']:
                content = header + new_sequence
            else:  # fastq format
                quality_scores = 'I' * len(sequence)  # Utilizzo di qualità fittizia
                content = f"@{ID_TRANSCRIPT_GENE} {ID_GENE}\n{new_sequence}\n+\n{quality_scores}"

            file.write(content + "\n")

    print(f"File {file_name}.{format} generato con successo.")
########################################################################################################################










##################################################### MAIN #############################################################
########################################################################################################################
if __name__ == '__main__':

    # Gestione argomenti ###############################################################################################
    parser = argparse.ArgumentParser()

    parser.add_argument('--type', dest='type', action='store', default='1f_np')
    parser.add_argument('--path', dest='path', action='store', default='basic/')
    parser.add_argument('--rev_comb', dest='rev_comb', action='store', default='false')
    parser.add_argument('--type_factorization', dest='type_factorization', action='store',default='CFL')
    parser.add_argument('--fasta', dest='fasta', action='store', default='transcript_genes.fa')
    parser.add_argument('--fingerprint', dest='fingerprint', action='store', default='prova_fingerprint.txt')
    parser.add_argument('--fact', dest='fact', action='store', default='create')
    parser.add_argument('--shift', dest='shift', action='store', default='shift')
    parser.add_argument('--filter', dest='filter', action='store', default='list')
    parser.add_argument('--split', dest='split', action='store', default=150, type=int)
    parser.add_argument('-n', dest='n', action='store', default=1, type=int)

    # Parameters for pseudo generate DNA
    parser.add_argument('--gc_content', type=float, required=False, help="GC content (between 0 and 1)")
    parser.add_argument('--format', required=False, choices=['fasta', 'fa', 'fastq'], help="Output file format")
    parser.add_argument('--size', type=int, required=False, help="Size of DNA sequence in bp")
    parser.add_argument('--number_dna_generate',type=int,required=False,help="Size of Number of DNA you want generate")


    args = parser.parse_args()

    if args.type == 'basic':
        print('\nFingerprint Step: 1f_np...\n')
        basic_fingerprint(args)


    elif args.type == 'generalized':
        print('\nFingerprint long reads...\n')
        generalized_fingerprint(args)


    elif args.type == 'mapping':
        print('\nMapping projecyion of fingerprint files...\n')
        fingerprint_mapping(args)
        

    elif args.type == 'generate':
        print(f"PATH FILE : {args.path}")
        # Make a for with number_dna_generate 
        sequence = generate_dna_sequences(args.number_dna_generate,args.size, args.gc_content)
        print("\n Success generate sequences... \n")
        generate_fasta_file(sequence, args.path, format=args.format)