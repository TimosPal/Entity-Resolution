#include "CliqueGroup.h"

#include <stdlib.h>
#include <stdio.h>

ItemCliquePair* ItemCliquePair_New(void* item){
    ItemCliquePair* pair = malloc(sizeof(ItemCliquePair)); //value of the KeyValuePair struct

    pair->clique = malloc(sizeof(List)); //malloc list for the clique of the pair
    List_Init(pair->clique); 

    pair->item = item;

    List_Append(pair->clique, pair); //append the pair to the clique(starting state)

    return pair;
}

void ItemCliquePair_Free(void* value){
    ItemCliquePair* icp = (ItemCliquePair*)value;
    
    free(icp);
}

void CliqueGroup_Init(CliqueGroup* cg, int bucketSize,unsigned int (*hashFunction)(const void*, unsigned int), bool (*cmpFunction)(void*, void*)){
    Hash_Init(&cg->hash, bucketSize, hashFunction, cmpFunction);
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
    /* Frees the entire structure (not the values)*/

    /* destroy hash and free ItemCliquePairs(not the items, just the struct)*/
    Hash_FreeValues(cg.hash, ItemCliquePair_Free);
    Hash_Destroy(cg.hash);
    /* Delete lists inside cliques list and destroy cliques list(which is on stack so no free) */
    List_FreeValues(cg.cliques, List_Free);
    List_Destroy(&(cg.cliques));
}

void CliqueGroup_FreeValues(CliqueGroup cg, void (*subFree)(void*)){
    /* Free all items in every list in cliques list*/
    Node* tempNode1 = cg.cliques.head;
    while (tempNode1 != NULL){
        List* insideList = (List*)(tempNode1->value);
        Node* tempNode2 = insideList->head;
        while (tempNode2 != NULL){
            ItemCliquePair* icp = (ItemCliquePair*)(tempNode2->value);
            subFree(icp->item);
            tempNode2 = tempNode2->next;
        }
        tempNode1 = tempNode1->next;
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
    if(icp1->clique == icp2->clique){
        return true;
    }

    List* mergedCliques = malloc(sizeof(List));
    List_Init(mergedCliques);

    /* NOTE: this is why we need the parent node in the ItemCliquePair(to remove the nodes from cliques list) */
    /* save old parent nodes to remove them from the cliques list later on, since they will be changed in MergeCliques*/
    Node* oldParentNode1 = icp1->cliqueParentNode;
    Node* oldParentNode2 = icp2->cliqueParentNode;
    /* save old cliques to destroy them after the merge and append is complete */
    List* oldClique1 = icp1->clique;
    List* oldClique2 = icp2->clique;

    List_Append(&cg->cliques, mergedCliques);
    CliqueGroup_MergeCliques(mergedCliques, *icp1->clique,*icp2->clique, cg->cliques.tail);


    /* remove the old parent nodes from the cliques list */
    List_RemoveNode(&cg->cliques, oldParentNode1);
    List_RemoveNode(&cg->cliques, oldParentNode2);
    
    /* destroy the old cliques */
    List_Destroy(oldClique1);
    List_Destroy(oldClique2);

    /* free the old cliques */
    free(oldClique1);
    free(oldClique2);

    return true;
}

void CliqueGroup_PrintIdentical(CliqueGroup* cg, void (*Print)(void* value)){
    Node* currClique = cg->cliques.head;
    while (currClique != NULL){
        // Printing each clique.
        List* currCliqueItems = (List*)(currClique->value);
        if (currCliqueItems->size > 1){ // Only printing cliques that contain 2+ items.

            // Print each similar pair.
            Node* currItemA = currCliqueItems->head;
            while (currItemA != NULL){
                Node* currItemB = currItemA->next;
                while(currItemB != NULL) {
                    ItemCliquePair *icpA = (ItemCliquePair *) (currItemA->value);
                    ItemCliquePair *icpB = (ItemCliquePair *) (currItemB->value);

                    Print(icpA->item);
                    printf(" ");
                    Print(icpB->item);
                    printf("\n");

                    currItemB = currItemB->next;
                }
                currItemA = currItemA->next;
            }

        }
        currClique = currClique->next;
    }
}

void CliqueGroup_MergeCliques(List* newList, List list1, List list2, Node* cliqueParentNode){
    /* merges 2 cliques into one and changes all the pointers of the ItemCliquePairs to the correct ones */
    // TODO: make merging faster and call half the updates on the pointers.
    List_Init(newList);

	Node* temp1 = list1.head;
	while(temp1 != NULL){
        ItemCliquePair* icp = (ItemCliquePair*)(temp1->value);
        icp->clique = newList;
        icp->cliqueParentNode = cliqueParentNode;
        List_Append(newList, icp);
		temp1 = temp1->next;
	}

	Node* temp2 = list2.head;
	while(temp2 != NULL){
        ItemCliquePair* icp = (ItemCliquePair*)(temp2->value);
        icp->clique = newList;
        icp->cliqueParentNode = cliqueParentNode;
        List_Append(newList, icp);
		temp2 = temp2->next;
	}

}