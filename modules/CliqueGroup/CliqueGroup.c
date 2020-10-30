#include "CliqueGroup.h"

#include <stdlib.h>

typedef struct ItemCliquePair {
    void* item;
    List* clique;
}ItemCliquePair;

ItemCliquePair* ItemCliquePair_New(void* item, List* clique){
    ItemCliquePair* pair = malloc(sizeof(ItemCliquePair));
    pair->clique = malloc(sizeof(List));
    pair->item = item;
}

void CliqueGroup_Init(CliqueGroup* cg, int bucketSize,unsigned int (*hashFunction)(void*, unsigned int), bool (*cmpFunction)(void*, void*)){
    Hash_Init(&cg->hash, bucketSize, hashFunction,cmpFunction);
    List_Init(&cg->cliques);
}

void CliqueGroup_Add(CliqueGroup* cg, void* value){
     ItemCliquePair_New()
}