# PRIMO APPROCCIO:
# PRENDERE DUE FILE FASTA E METTERLI IN UN UNICO FILE
# EFFETTUARE (LONGEST AND MOST FREQUENT TCOMMON SEQUENCE)
    # Estraiamo tutte le sottosequenze possibili per entrambe le sequenze.
    # Confrontiamo le sottosequenze delle due sequenze per trovare quelle comuni.
    # Tracciamo quante volte ogni sottosequenza comune si ripete tra le due sequenze.
    # Identifichiamo la sottosequenza con la frequenza totale maggiore.
    # Alla fine, selezioniamo la sottosequenza comune che è sia la più lunga sia la più frequente. 
    # In caso di parità di lunghezza, scegliamo quella con la maggiore frequenza. (DA FARE)
    # Altrimenti prendiamo la prima in ordine alfabetico. (DA FARE)
# EFFETTUARE LO SPLIT DEI DUE FILE ORIGINALI TROVANDO LA SEQUENZA
# SPLITTARE DAL SUO INIZIO AGGIUNGENDO A CAPO RIGA DI ID E POI A CAPO LA NUOVA SEQUENZA


# FARE IL COMANDO DA CONSOLE


#APPROCCIO CON SUFFIX ARRAY
import sys

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

def find_all_common_substrings_with_frequencies(seq1, seq2):
    combined_seq = seq1 + '#' + seq2 + '$'
    s1_len = len(seq1)


    suffix_array = build_suffix_array(combined_seq)
    

    lcp = build_lcp(combined_seq, suffix_array)

    substrings_with_frequencies = []


    for i in range(1, len(combined_seq)):
        if (suffix_array[i] < s1_len) != (suffix_array[i - 1] < s1_len):
            if lcp[i] >= 4: 
                substring = combined_seq[suffix_array[i]:suffix_array[i] + lcp[i]]
                
               
                frequency1 = seq1.count(substring)
                frequency2 = seq2.count(substring)
                
               
                substrings_with_frequencies.append((substring, frequency1, frequency2))

    # le sottosequenze vengono salvate in ordine decrescente e vengono ordinate
    # in modo tale che la più lunga e la più frequente (e la prima in ordine alfabetico) sia la scelta (DA CONCORDARE)
    substrings_with_frequencies.sort(key=lambda x: (len(x[0]), x[1] + x[2]), reverse=True)

    return substrings_with_frequencies

def save_substrings_to_file(substrings_with_frequencies, output_file):
    try:
        with open(output_file, 'w') as f:
            for substring, freq1, freq2 in substrings_with_frequencies:
                f.write(f"Sottosequenza: {substring}, Frequenza in file1: {freq1}, Frequenza in file2: {freq2}\n")
        print(f"File {output_file} salvato con successo.")
    except IOError:
        print(f"Errore durante il salvataggio del file {output_file}")



def LMFCS(file1, file2, output_file):
    try:
        with open(file1, 'r') as f1:
            seq1 = f1.read()
    except IOError:
        print(f"Errore durante l'apertura del file {file1}")
        return
    
    try:
        with open(file2, 'r') as f2:
            seq2 = f2.read()
    except IOError:
        print(f"Errore durante l'apertura del file {file2}")
        return

   
    substrings_with_frequencies = find_all_common_substrings_with_frequencies(seq1, seq2)
    
    
    save_substrings_to_file(substrings_with_frequencies, output_file)

    
    if substrings_with_frequencies:
        best_choice = substrings_with_frequencies[0]
        print(f"La migliore sottosequenza è: {best_choice[0]} con frequenza in file1: {best_choice[1]} e in file2: {best_choice[2]}")
    else:
        print("Nessuna sottosequenza comune trovata di lunghezza >= 4.")

    return best_choice[0]


if __name__ == '__main__':
    file1 = "lcs_fasta/file1.fasta"
    file2 = "lcs_fasta/file2.fasta"
    output_file = "substrings_frequencies.txt"
    
    substring = LMFCS(file1, file2, output_file)
