#include "Hash.h"

#include <stdlib.h>

KeyValuePair* KeyValuePair_Create(void* key, void* value){
    KeyValuePair* kvp = malloc(sizeof(KeyValuePair));

    kvp->key = key;
    kvp->value = value;

    return kvp;
}

void Hash_Init(Hash* hash, int bucketSize, unsigned int (*hashFunction)(void*), bool (*cmpFunction)(void*, void*)) {
    hash->bucketSize = bucketSize; // size of the array.
    hash->hashFunction = hashFunction; // used in get.
    hash->cmpFunction = cmpFunction; // used in get.
    hash->buckets = malloc(bucketSize * sizeof(List));

    for(int i = 0; i < bucketSize; i++)
        List_Init(&hash->buckets[i]);

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
    int index = hash->hashFunction(key);
    List *bucketItems = &(hash->buckets[index]);

    KeyValuePair *kvp = KeyValuePair_Create(key, value);
    List_Append(bucketItems, kvp); 
}

void Hash_Destroy(Hash hash){
    for(int i = 0; i < hash.bucketSize; i++){
        List_Destroy(&(hash.buckets[i]));
    }
    free(hash.buckets);
}