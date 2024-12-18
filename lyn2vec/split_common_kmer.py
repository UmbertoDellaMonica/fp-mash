# find common kmers between two files, saves them in this type of file
# kmer      frequenza in file1      frequenza in file2
# split the two initial file based on ALL common kmers

def findcommon(file1, file2, size, common_kmer):
    # i file di dsk sono tutti del tipo kmer e frequenza:
        # prendere i primi size caratteri e confrontarli er trovare i comuni
        # se li trovano allora mettere in un nuovo file e aggiungere ciò che sta scritto a size + 1 di entrambi i file
        with open(file1, 'r') as file1, open(file2, 'r') as file2, open(common_kmer, 'w+') as output:
            for line in file1:
                file2.seek(0)
                for line2 in file2:
                    if line[0:size] == line2[0:size]:
                         output.writelines(line[0:size] + " " +  line[size:].strip() + " " +  line2[size:].strip())
                         output.write("\n")
            return output


def reverse_and_complement(file,size, f1, f2):

    with open(file, 'r') as file, open("rc_common.txt", 'w') as output, open(f1, 'r') as f1, open(f2, 'r') as f2:
        for line in file:
            kmer = ""
            # reverse
            line_rev = line[0:size][::-1]
            # operazione di complemento
            for i in range(size):
                if line_rev[i] == 'A': kmer+='T'
                if line_rev[i] == 'C': kmer+='G'
                if line_rev[i] == 'G': kmer+='C'
                if line_rev[i] == 'T': kmer+='A'
            
            for kmers in f1:
               if (kmers.find(line))>1 : output.writelines(line[0:size] + line[size:]) # ordine lessicografico serve?

            else:
                output.writelines(kmer + line[size:])
                
            
def split_files(input_file, kmer_list_file, size, num):
    with open(kmer_list_file, 'r') as kmer_file:
        kmer_set = set(kmer_file.read(size).split())

    with open(input_file, 'r') as infile, open("splitted_" + num + ".txt", 'w') as outfile:
        current_id = None
        buffer = ""

        for line in infile:
            if line.startswith(">"):
                if buffer:
                    write_split_sequence(outfile, current_id, buffer, kmer_set)
                    buffer = ""
                current_id = line.strip()
            else:
                buffer += line.strip()

        #ultimo buffer rimasto
        if buffer:
            write_split_sequence(outfile, current_id, buffer, kmer_set)


def write_split_sequence(outfile, current_id, sequence, kmer_set):

    start = 0
    while start < len(sequence):
        match_found = False

        for kmer in kmer_set:
            index = sequence.find(kmer, start)
            if index != -1:
                if start < index:
                    outfile.write(current_id + "\n" + sequence[start:index] + "\n")
                # Scrivi il kmer insieme alla parte rimanente della sequenza
                outfile.write(current_id + "\n" + sequence[index:] + "\n")
                start = len(sequence)  # Termina il loop per questa sequenza
                match_found = True
                break

        if not match_found:
            outfile.write(current_id + "\n" + sequence[start:] + "\n")
            break

            
                

if __name__ == '__main__':
    file1 = "../dsk/bin/output.txt"
    file2 = "../dsk/bin/output2.txt"
    kmersize = 10
    common_kmer= "common_kmers.txt"

    findcommon(file1, file2, kmersize, common_kmer)
    reverse_and_complement(common_kmer, kmersize, file1, file2)

    split_files("lmfcs_fasta/file1.fasta", "rc_common.txt", kmersize, "1")
    split_files("lmfcs_fasta/file2.fasta", "rc_common.txt", kmersize, "2")