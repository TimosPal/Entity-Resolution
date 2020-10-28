#include "Hash.h"

#include <stdlib.h>

void Hash_Init(Hash* hash, int bucketSize, unsigned int (*hashFunction)(void*), bool (*cmpFunction)(void*, void*)) {
    hash->bucketSize = bucketSize; // size of the array.
    hash->hashFunction = hashFunction; // used in get.
    hash->cmpFunction = cmpFunction; // used in get.
    hash->buckets = malloc(bucketSize * sizeof(List));

    for(int i = 0; i < bucketSize; i++)
        List_Init(&hash->buckets[i]);

    List_Init(&hash->unorderedPairs);
}

void* Hash_GetValue(Hash hash,void* key){
    int index = hash.hashFunction(key);
    List bucketItems = hash.buckets[index];

    Node* currNode = bucketItems.head;
    while(currNode != NULL){
        KeyValuePair* pair = (KeyValuePair*)currNode->value;
        if(hash.cmpFunction(key,pair->key)){
            return pair->value;
        }

        currNode = currNode->next;
    }

    return NULL;
}

void Hash_Add(Hash* hash,void* key,void* value){
    
}