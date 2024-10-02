def distance_between_hash_lists(list1, list2):
    """Calcola la distanza tra due liste di hash. 
    La distanza viene calcolata come il numero di elementi che non corrispondono."""
    # Converte le liste in set per facilitare il calcolo della distanza
    set1 = set(list1)
    set2 = set(list2)
    # Calcola gli elementi unici in ciascuna lista
    unique_in_set1 = set1 - set2
    unique_in_set2 = set2 - set1
    # Restituisce la distanza come somma degli elementi unici
    return len(unique_in_set1) + len(unique_in_set2)

def are_hash_lists_similar(list1, list2):
    # Calcola la distanza tra le due liste
    distance = distance_between_hash_lists(list1, list2)
    
    # Determina il numero totale di bit basato sulla dimensione delle liste
    total_bits = max(len(list1), len(list2)) * 32  # Supponiamo che ogni hash sia a 32 bit
    
    # Calcola la tolleranza come il 25% del totale dei bit
    threshold = total_bits * 0.25  # 25% del totale dei bit

    # Restituisce True se la distanza è inferiore o uguale alla tolleranza
    return distance <= (total_bits - threshold)







def jaccard_similarity_and_common(set1, set2):
        intersection_size = 0
        union_size = 0
        total_common = 0
        total_denom = 0

        larger_set = set1 if len(set1) > len(set2) else set2
        smaller_set = set1 if len(set1) <= len(set2) else set2

        found_indices = set()

        for list1 in larger_set:
            found_similar = False
            for list2 in smaller_set:
                if are_hash_lists_similar(list1, list2):
                    intersection_size += 1
                    total_common += 1
                    found_similar = True
                    found_indices.add(tuple(list1))  # Using tuple for set
                    break
            union_size += 1
            total_denom += 1

        for list2 in smaller_set:
            found_similar = False
            for list1 in larger_set:
                if are_hash_lists_similar(list1, list2):
                    found_similar = True
                    break
            if not found_similar:
                union_size += 1
                total_denom += 1

        if union_size == 0:
            return 0.0, total_common, total_denom

        jaccard_similarity = intersection_size / union_size
        return jaccard_similarity, total_common, total_denom

if __name__ == "__main__":


        # Definizione degli insiemi di Hash
    set1 = [
        [885162080, 4288887283, 2096355651, 1740219326, 1508569014, 317738052],
        [317738052, 4288887283, 2096355651, 1740219326, 1508569014, 885162080],
        [4288887283, 2096355651, 1740219326, 1508569014, 1460359472],
        [317738052, 3787528170, 2301948614, 2096355651, 1740219326, 1508569014, 3282828596],
        [885162080, 3282828596, 1508569014, 3848151429, 317738052]
    ]

    set2 = [
        [885162080, 4288887283, 2096355651, 1740219326, 1508569014, 317738052],
        [317738052, 4288887283, 2096355651, 1740219326, 1508569014, 885162080],
        [4288887283, 2096355651, 1740219326, 1508569014, 1460359472],
        [317738052, 3787528170, 2301948614, 2096355651, 1740219326, 1508569014, 3282828596],
        [3787528170, 2301948614, 2096355651, 1740219326, 1508569014, 3282828596, 317738052]
    ]

    # Calcolo della similarità di Jaccard
    similarity, total_common, total_denom = jaccard_similarity_and_common(set1, set2)

    # Stampare i risultati
    print(f"Distanza di Jaccard: {similarity}")
    print(f"Elementi comuni totali: {total_common}")
    print(f"Denominatore totale (elementi unici): {total_denom}")
