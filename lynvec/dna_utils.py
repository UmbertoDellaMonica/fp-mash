import random
import datetime
import string

import random

def make_dna(length, gc_content):
    """
    Genera una sequenza di DNA casuale.

    Args:
        length (int): La lunghezza della sequenza di DNA in bp.
        gc_content (float): La percentuale di contenuto GC (tra 0 e 1).
    
    Returns:
        str: La sequenza di DNA generata in maiuscolo.
    """
    if not (0 <= gc_content <= 1):
        raise ValueError("Il contenuto GC deve essere tra 0 e 1.")

    sequence = []
    
    for _ in range(length):
        if random.random() < gc_content:
            # Scegli tra G e C con probabilità uguale
            base = 'G' if random.random() < 0.5 else 'C'
        else:
            # Scegli tra A e T con probabilità uguale
            base = 'A' if random.random() < 0.5 else 'T'
        
        sequence.append(base)
    
    ID = ''.join(sequence)
    return ID.upper()



def generate_transcript_short_id(length=8):
    """
    Genera un ID pseudocasuale corto con il carattere 'T' all'inizio.

    Args:
        length (int): La lunghezza dell'ID generato, escluso il carattere iniziale 'T'. Predefinito è 8.
    
    Returns:
        str: Un ID pseudocasuale corto con 'T' all'inizio.
    """
    characters = string.ascii_letters + string.digits
    random_id = ''.join(random.choice(characters) for _ in range(length))
    return 'T00000' + random_id.upper()



def generate_genes_short_id(id_string):
    """
    Sostituisce il primo carattere di una stringa con 'G'.

    Args:
        id_string (str): L'ID originale.
    
    Returns:
        str: L'ID modificato con 'G' come primo carattere.
    """
    if not id_string:
        raise ValueError("L'ID fornito è vuoto.")
    
    return 'G' + id_string[1:]



def generate_dna_sequences(num_sequences, size, gc_content):
    """
    Genera una lista di sequenze di DNA casuali.

    Args:
        num_sequences (int): Il numero di sequenze di DNA da generare.
        size (int): La lunghezza di ciascuna sequenza di DNA in bp.
        gc_content (float): La percentuale di contenuto GC (tra 0 e 1).
    
    Returns:
        list of str: Una lista di sequenze di DNA generate.
    """
    return [make_dna(size, gc_content) for _ in range(num_sequences)]