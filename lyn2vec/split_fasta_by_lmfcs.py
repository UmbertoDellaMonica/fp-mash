# PRIMO APPROCCIO:
# PRENDERE DUE FILE FASTA E METTERLI IN UN UNICO FILE
# EFFETTUARE (LONGEST AND MOST FREQUENT TCOMMON SEQUENCE) 
    # Estraiamo tutte le sottosequenze possibili per entrambe le sequenze.
    # Confrontiamo le sottosequenze delle due sequenze per trovare quelle comuni.
    # Tracciamo quante volte ogni sottosequenza comune si ripete tra le due sequenze.
    # Identifichiamo la sottosequenza con la frequenza totale maggiore.
    # Alla fine, selezioniamo la sottosequenza comune che è sia la più lunga sia la più frequente. 
    # In caso di parità di lunghezza, scegliamo quella con la maggiore frequenza. 
    # Altrimenti prendiamo la prima in ordine alfabetico.
    # (CONCORDARE DUE PUNTI):
        # I PARAMETRI DA SCEGLIERE PER CAPIRE QUALE LUNGHEZZA DELLA SOTTOSEQUENZA PRENDERE
        # CAPIRE IL TIPO DI FREQUENZA PREFERIBILE PER LA FATTORIZZAZIONE
# EFFETTUARE LO SPLIT DEI DUE FILE ORIGINALI TROVANDO LA SEQUENZA 
# SPLITTARE DAL SUO INIZIO AGGIUNGENDO A CAPO RIGA DI ID E POI A CAPO LA NUOVA SEQUENZA
# FARE IL COMANDO DA CONSOLE 


#APPROCCIO CON SUFFIX ARRAY
import sys
import argparse

def build_suffix_array(s):
    suffixes = sorted((s[i:], i) for i in range(len(s)))
    return [suffix[1] for suffix in suffixes]

def build_lcp(s, suffix_array):
    n = len(s)
    rank = [0] * n
    lcp = [0] * n    
    for i, suffix in enumerate(suffix_array):
        rank[suffix] = i
    h = 0
    for i in range(n):
        if rank[i] > 0:
            j = suffix_array[rank[i] - 1]
            while i + h < n and j + h < n and s[i + h] == s[j + h]:
                h += 1
            lcp[rank[i]] = h
            if h > 0:
                h -= 1
    return lcp

def find_all_common_substrings_with_frequencies(seq1, seq2, min_length):
    combined_seq = seq1 + '#' + seq2 + '$'
    s1_len = len(seq1)
    suffix_array = build_suffix_array(combined_seq)
    lcp = build_lcp(combined_seq, suffix_array)
    substrings_with_frequencies = []
    for i in range(1, len(combined_seq)):
        if (suffix_array[i] < s1_len) != (suffix_array[i - 1] < s1_len):
            if lcp[i] >= min_length: # parametro da mettere da linea di comando
                substring = combined_seq[suffix_array[i]:suffix_array[i] + lcp[i]]               
                frequency1 = seq1.count(substring)
                frequency2 = seq2.count(substring)               
                substrings_with_frequencies.append((substring, frequency1, frequency2))

    substrings_with_frequencies.sort(key=lambda x: (-len(x[0]), -(x[1] + x[2]), x[0]))
    return substrings_with_frequencies

def save_substrings_to_file(substrings_with_frequencies, output_file):
    try:
        with open(output_file, 'w') as f:
            for substring, freq1, freq2 in substrings_with_frequencies:
                f.write(f"Sottosequenza: {substring}, Frequenza in file1: {freq1}, Frequenza in file2: {freq2}\n")
        print(f"File {output_file} salvato con successo.")
    except IOError:
        print(f"Errore durante il salvataggio del file {output_file}")

def read_fasta_sequence(file_path):
    sequence = []
    try:
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()
                if not line.startswith('>'):  
                    sequence.append(line)
        if not sequence:
            print(f"Il file {file_path} non contiene sequenze valide.")
            return None
        return ''.join(sequence)  
    except IOError:
        print(f"Errore durante l'apertura del file {file_path}")
        return None


def LMFCS(file1, file2, output_file, min_length):
    seq1 = read_fasta_sequence("lcs_fasta/" + file1)
    seq2 = read_fasta_sequence("lcs_fasta/" + file2)

    if seq1 is None or seq2 is None:
        print("Errore nel caricamento dei file.")
        return None

    substrings_with_frequencies = find_all_common_substrings_with_frequencies(seq1, seq2, min_length)
    save_substrings_to_file(substrings_with_frequencies, output_file)

    if substrings_with_frequencies:
        best_choice = substrings_with_frequencies[0]
        print(f"La migliore sottosequenza è: {best_choice[0]} con frequenza in file1: {best_choice[1]} e in file2: {best_choice[2]}")
        return best_choice[0]
    else:
        print("Nessuna sottosequenza comune trovata.")
        return None

def split_sequence(sequence, subseq):
    result = []
    start = 0
    
    while True:
        index = sequence.find(subseq, start)
        
        if index == -1:
            result.append(sequence[start:])
            break
        

        result.append(sequence[start:index + len(subseq)])
        start = index + len(subseq)
    
    return result


def split_file(filename, substring):
    if substring is None:
        print(f"Nessuna sottosequenza trovata per dividere {filename}.")
        return

    with open("lcs_fasta/" + filename, 'r') as f_in, open("lcs_fasta/splitted_" + filename, 'w') as f_out:
        current_id = None
        current_sequence = ""

        for line in f_in:
            line = line.strip()
            if line.startswith('>'):
                if current_id and current_sequence:
                    chunks = split_sequence(current_sequence, substring)
                    for chunk in chunks:
                        f_out.write(f"{current_id}\n")  
                        f_out.write(f"{chunk}\n")  
                current_id = line 
                current_sequence = ""  
            else:
                current_sequence += line  

        
        if current_id and current_sequence:
            chunks = split_sequence(current_sequence, substring)
            for chunk in chunks:
                f_out.write(f"{current_id}\n")
                f_out.write(f"{chunk}\n")


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    
    parser.add_argument('--file1', required = True)
    parser.add_argument('--file2', required = True)
    parser.add_argument('--output', required = False, default="substrings_frequencies.txt")
    parser.add_argument('--min_length', type=int)

    args = parser.parse_args()

    file1 = args.file1
    file2 = args.file2
    output_file = "lcs_fasta/" + args.output
    min_length = args.min_length

    
    # Esegui il calcolo della sottosequenza più lunga e frequente
    substring = LMFCS(file1, file2, output_file, min_length)

    # Effettua lo split dei file originali utilizzando la sottosequenza trovata
    split_file(file1, substring)
    split_file(file2, substring)



