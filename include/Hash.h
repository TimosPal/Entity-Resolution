#ifndef HASH_H
#define HASH_H

#include "LinkedList.h"

#include <stdbool.h>

typedef struct Hash {
    List* buckets;
    List keyValuePairs;

    unsigned int (*hashFunction)(const void*, unsigned int);
    bool (*cmpFunction)(void*,void*);

    int bucketSize;
} Hash;

typedef struct KeyValuePair{
    void* key;
    void* value;
}KeyValuePair;


void Hash_Init(Hash* hash, int bucketSize, unsigned int (*hashFunction)(const void*, unsigned int), bool (*cmpFunction)(void*, void*));
void* Hash_GetValue(Hash hash,void* key,int keySize); //Returns a value based on a key.
void Hash_FreeValues(Hash hash,void (*freeMethod)(void*));
void Hash_Destroy(Hash hash);
bool Hash_Add(Hash* hash,void* key,int keySize,void* value);


#endif
