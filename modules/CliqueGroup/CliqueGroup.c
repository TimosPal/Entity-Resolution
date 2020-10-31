#include "CliqueGroup.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct ItemCliquePair {
    void* item;
    List* clique;
    Node* cliqueParentNode;
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
    icp->cliqueParentNode = cg->cliques.tail;

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

bool CliqueGroup_Update(CliqueGroup* cg, void* key1, int keySize1, void* key2, int keySize2){
    ItemCliquePair* icp1 = Hash_GetValue(cg->hash, key1, keySize1);
    if(icp1 == NULL)
        return false;
    ItemCliquePair* icp2 = Hash_GetValue(cg->hash, key2, keySize2);
    if(icp2 == NULL)
        return false;

    // If both icps point to the same list then they are already in the same clique.
    // So no further changes should be made.
    if(icp1->clique == icp2->clique)
        return true;

    List* mergedCliques = malloc(sizeof(List));
    *mergedCliques = List_Merge(*icp1->clique,*icp2->clique);
    List_Append(&cg->cliques, mergedCliques);

    // Testing.
    List_RemoveNode(&cg->cliques, icp1->cliqueParentNode);
    List_RemoveNode(&cg->cliques, icp2->cliqueParentNode);
    

    icp1->cliqueParentNode = cg->cliques.tail;
    icp2->cliqueParentNode = cg->cliques.tail;

    List_Destroy(icp1->clique);
    //free(icp1->clique);
    List_Destroy(icp2->clique);
    //free(icp2->clique);

    icp1->clique = mergedCliques;
    icp2->clique = mergedCliques;

    return true;
}