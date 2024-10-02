class HashList:
    def __init__(self, use64=True):
        self.use64 = use64
        self.hashes64 = []
        self.hashes32 = []

    def add(self, hash_value):
        if self.use64:
            self.hashes64.append(hash_value)
        else:
            self.hashes32.append(hash_value)

    def to_list(self):
        if self.use64:
            return self.hashes64
        else:
            # Conversione dei valori 32-bit in 64-bit (se necessario)
            return [hash_value for hash_value in self.hashes32]
