#include "Hash.h"

#include <stdlib.h>

KeyValuePair* KeyValuePair_Create(void* key, void* value){
    KeyValuePair* kvp = malloc(sizeof(KeyValuePair));

    kvp->key = key;
    kvp->value = value;

    return kvp;
}

void Hash_Init(Hash* hash, int bucketSize, unsigned int (*hashFunction)(void*,int), bool (*cmpFunction)(void*, void*)) {
    hash->bucketSize = bucketSize; // size of the array.
    hash->hashFunction = hashFunction; // used in get.
    hash->cmpFunction = cmpFunction; // used in get.
    hash->buckets = malloc(bucketSize * sizeof(List));

    for(int i = 0; i < bucketSize; i++)
        List_Init(&hash->buckets[i]);

}

void* Hash_GetValue(Hash hash,void* key,int keySize){
    unsigned int index = hash.hashFunction(key,keySize) % hash.bucketSize;
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

bool Hash_Add(Hash* hash,void* key,int keySize,void* value){
    void* val = Hash_GetValue(*hash,key,keySize);
    if(val != NULL)
        return false;

    unsigned int index = hash->hashFunction(key,keySize) % hash->bucketSize;
    List *bucketItems = &(hash->buckets[index]);

    KeyValuePair *kvp = KeyValuePair_Create(key, value);
    List_Append(bucketItems, kvp);

    return true;
}

void Hash_Destroy(Hash hash){
    for(int i = 0; i < hash.bucketSize; i++){
        List_Destroy(&(hash.buckets[i]));
    }

    free(hash.buckets);
}

void Hash_FreeValues(Hash hash,void (*freeMethod)(void*)){
    for(int i = 0; i < hash.bucketSize; i++){
        freeMethod(&(hash.buckets[i]));
    }
}