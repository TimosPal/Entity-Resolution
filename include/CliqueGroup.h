#ifndef CLIQUEGROUP_H
#define CLIQUEGROUP_H

#include "Hash.h"
#include "LinkedList.h"
#include <stdbool.h>

/* NOTE: this structure supposes that cliques list is a list of lists */
typedef struct CliqueGroup{
    Hash hash;
    List cliques;
} CliqueGroup;

void CliqueGroup_Init(CliqueGroup* cg, int bucketSize,unsigned int (*hashFunction)(const void*, unsigned int), bool (*cmpFunction)(void*, void*));
void CliqueGroup_Destroy(CliqueGroup cg);
void CliqueGroup_FreeValues(CliqueGroup cg, void (*subFree)(void*));
void CliqueGroup_PrintIdentical(CliqueGroup* cg, void (*Print)(void* value));

void CliqueGroup_MergeCliques(List* newList, List list1, List list2, Node* cliqueParentNode);

bool CliqueGroup_Add(CliqueGroup* cg, void* key, int keySize, void* value);
bool CliqueGroup_Update(CliqueGroup* cg, void* key1, int keySize1, void* key2, int keySize2);


#endif
