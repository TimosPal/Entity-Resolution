#include "CliqueGroup.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct ItemCliquePair {
    void* item;
    List* clique;
}ItemCliquePair;

ItemCliquePair* ItemCliquePair_New(void* item){
    ItemCliquePair* pair = malloc(sizeof(ItemCliquePair)); //value of the KeyValuePair struct

    pair->clique = malloc(sizeof(List)); //malloc list for the clique of the pair
    List_Init(pair->clique); 

    List_Append(pair->clique, item); //append the item to the clique(starting state)

    pair->item = item;

    return pair;
}

void ItemCliquePair_Free(void* value){
    ItemCliquePair* icp = (ItemCliquePair*)value;
    
    List_Destroy((List*)icp->clique);
    free(icp->clique);
    free(icp);
}

void CliqueGroup_Init(CliqueGroup* cg, int bucketSize,unsigned int (*hashFunction)(const void*, unsigned int), bool (*cmpFunction)(void*, void*)){
    Hash_Init(&cg->hash, bucketSize, hashFunction,cmpFunction);
    List_Init(&cg->cliques);
}

bool CliqueGroup_Add(CliqueGroup* cg, void* key, int keySize, void* value){
    if (Hash_GetValue(cg->hash, key, keySize) != NULL){
        return false;
    }

    ItemCliquePair* icp = ItemCliquePair_New(value); //create icp
    Hash_Add(&(cg->hash), key, keySize, icp);

    List_Append(&(cg->cliques), icp->clique); //append the clique into the list of cliques

    return true;
}

void CliqueGroup_Destroy(CliqueGroup cg){
    /* Frees the entire structure */

    /* free hash (including the lists inside cliques list) */
    Hash_FreeValues(cg.hash, ItemCliquePair_Free);
    Hash_Destroy(cg.hash);
    /* Delete cliques list */
    List_Destroy(&(cg.cliques));
}

void CliqueGroup_FreeValues(CliqueGroup cg, void (*subFree)(void*)){
    /* For every list in cliques list, frees values in them */
    Node* tempNode = cg.cliques.head;
    while (tempNode != NULL){
        List_FreeValues(*(List*)(tempNode->value), subFree);
        tempNode = tempNode->next;
    }
}
