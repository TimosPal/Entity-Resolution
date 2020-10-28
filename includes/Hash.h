#ifndef HASH_H
#define HASH_H

#include "List.h"

#include <stdbool.h>

typedef struct Hash {
    List* buckets;

    unsigned int (*hashFunction)(void*);
    bool (*cmpFunction)(void*,void*);

    int bucketSize;
} Hash;

typedef struct KeyValuePair{
    void* key;
    void* value;
}KeyValuePair;

void Hash_Init(Hash* hash, int bucketSize, unsigned int (*hashFunction)(void*), bool (*cmpFunction)(void*, void*));
void* Hash_GetValue(Hash hash,void* key); //Returns a value based on a key.


#endif
