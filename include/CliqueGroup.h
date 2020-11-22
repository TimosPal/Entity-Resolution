#ifndef CLIQUEGROUP_H
#define CLIQUEGROUP_H

#include "Hash.h"
#include "LinkedList.h"
#include <stdbool.h>

typedef struct Clique{
    List similar;
    List nonSimilar;
    Hash* nonSimilarHash;
}Clique;

typedef struct ItemCliquePair {
    void* item;
    Clique* clique;
    Node* cliqueParentNode;
}ItemCliquePair;

/* NOTE: this structure supposes that cliques list is a list of lists */
typedef struct CliqueGroup{
    Hash hash;
    List cliques;
    bool finalizeNeeded;
}CliqueGroup;


void CliqueGroup_Init(CliqueGroup* cg, int bucketSize,unsigned int (*hashFunction)(const void*, unsigned int), bool (*cmpFunction)(void*, void*));
void CliqueGroup_Destroy(CliqueGroup cg);
void CliqueGroup_FreeValues(CliqueGroup cg, void (*subFree)(void*));
void CliqueGroup_PrintIdentical(CliqueGroup* cg, void (*Print)(void* value));

void CliqueGroup_MergeCliques(Clique* newClique, Clique clique1, Clique clique2, Node* cliqueParentNode);

bool CliqueGroup_Add(CliqueGroup* cg, void* key, int keySize, void* value);
bool CliqueGroup_Update_Similar(CliqueGroup* cg, void* key1, int keySize1, void* key2, int keySize2);
bool CliqueGroup_Update_NonSimilar(CliqueGroup* cg, void* key1, int keySize1, void* key2, int keySize2);
void CliqueGroup_Finalize(CliqueGroup cg);

ItemCliquePair* ItemCliquePair_New(void* item);
void ItemCliquePair_Free(void* value);

Clique* Clique_New();
void Clique_Free(void* value);


#endif