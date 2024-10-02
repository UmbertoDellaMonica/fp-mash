# main.py

from hash_list import HashList

def jaccard_similarity_and_common(set1, set2):
    union_set = set()
    intersection_set = set()

    # Calcolare l'unione e l'intersezione per il primo set
    for hash_list in set1:
        current_set = set(hash_list.to_list())
        union_set.update(current_set)
        if not intersection_set:
            intersection_set = current_set
        else:
            intersection_set.intersection_update(current_set)

    # Calcolare l'unione e l'intersezione per il secondo set
    for hash_list in set2:
        current_set = set(hash_list.to_list())
        union_set.update(current_set)
        if not intersection_set:
            intersection_set = current_set
        else:
            intersection_set.intersection_update(current_set)

    total_common = len(intersection_set)
    total_denom = len(union_set)

    # Calcolare la distanza di Jaccard
    if total_denom == 0:
        return 1.0, total_common, total_denom  # Distanza massima se entrambi gli insiemi sono vuoti

    jaccard_index = total_common / total_denom
    jaccard_distance = 1.0 - jaccard_index

    return jaccard_distance, total_common, total_denom


if __name__ == "__main__":
    # Funzione per inserire gli hash in un HashList
    def create_hashlist_from_input():
        hash_list = HashList(use64=True)
        while True:
            value = input("Inserisci un hash (o 'fine' per terminare): ")
            if value.lower() == 'fine':
                break
            try:
                # Aggiungere l'hash come intero
                hash_list.add(int(value))
            except ValueError:
                print("Valore non valido. Per favore, inserisci un numero intero.")
        return hash_list

    # Richiedere all'utente il numero di HashList da creare per set1
    num_hash_lists_set1 = int(input("Quanti HashList vuoi creare per set1? "))
    set1 = []
    print("Inserisci gli elementi per set1:")
    for i in range(num_hash_lists_set1):
        print(f"Inserisci HashList {i + 1} per set1:")
        set1.append(create_hashlist_from_input())

    # Richiedere all'utente il numero di HashList da creare per set2
    num_hash_lists_set2 = int(input("Quanti HashList vuoi creare per set2? "))
    set2 = []
    print("Inserisci gli elementi per set2:")
    for i in range(num_hash_lists_set2):
        print(f"Inserisci HashList {i + 1} per set2:")
        set2.append(create_hashlist_from_input())

    # Calcolare la distanza di Jaccard e i totali comuni
    distance, total_common, total_denom = jaccard_similarity_and_common(set1, set2)

    # Stampare i risultati
    print(f"Distanza di Jaccard: {distance}")
    print(f"Elementi comuni totali: {total_common}")
    print(f"Denominatore totale (elementi unici): {total_denom}")
