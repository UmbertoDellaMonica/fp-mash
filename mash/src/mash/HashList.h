#ifndef HashList_h
#define HashList_h

#include "hash.h"
#include <vector>
#include <iostream>
#include <string>

class HashList
{
public:
    
    HashList() {use64 = true;}
    HashList(bool use64new) {use64 = use64new;}
    
    hash_u at(int index) const;
    void clear();
    void resize(int size);
    void set32(int index, uint32_t value);
    void set64(int index, uint64_t value);
    void setUse64(bool use64New) {use64 = use64New;}
    int size() const {return use64 ? hashes64.size() : hashes32.size();}
    void sort();
    void push_back32(hash32_t hash) {hashes32.push_back(hash);}
    void push_back64(hash64_t hash) {hashes64.push_back(hash);}
    bool get64() const {return use64;}

    // Nuovo metodo add
    void add(const hash_u& hash) {
        if (use64) {
            hashes64.push_back(hash.hash64);
        } else {
            hashes32.push_back(hash.hash32);
        }
    }

    std::vector<uint64_t> toVector() const {
        if (use64) {
            return std::vector<uint64_t>(hashes64.begin(), hashes64.end());
        } else {
            std::vector<uint64_t> result;
            result.reserve(hashes32.size());
            for (const auto& hash : hashes32) {
                result.push_back(static_cast<uint64_t>(hash)); // O una conversione adeguata
            }
            return result;
        }
    }


    // Funzione per stampare il contenuto di HashList
    std::string toString() const {
        std::string result = "{ ";
        if (use64) {
            for (const auto& hash : hashes64) {
                result += std::to_string(hash) + " ";
            }
        } else {
            for (const auto& hash : hashes32) {
                result += std::to_string(hash) + " ";
            }
        }
        result += "}";
        return result;
    }



private:
    
    bool use64;
    std::vector<hash32_t> hashes32;
    std::vector<hash64_t> hashes64;


    
};

#endif
