
#include "CliqueGroup.h"

#include <stdlib.h>
#include <stdio.h>

#include "Tuple.h"
#include "Hash.h"
#include "Hashes.h"
#include "Util.h"
#include "StringUtil.h"

ItemCliquePair* ItemCliquePair_New(void* item){
    static unsigned int id = 0;

    ItemCliquePair* pair = malloc(sizeof(ItemCliquePair)); //value of the KeyValuePair struct

    pair->clique = Clique_New();
    
    //set auto-incrementing id
    pair->id = id++;

    pair->item = item;

    List_Append(&pair->clique->similar, pair);

    return pair;
}

void ItemCliquePair_Free(void* value){
    ItemCliquePair* icp = (ItemCliquePair*)value;
    
    free(icp);
}

Clique* Clique_New(){
    static unsigned int id = 0;

    Clique* clique = malloc(sizeof(Clique));

    clique->id = id++;
    List_Init(&clique->similar);
    List_Init(&clique->nonSimilar);
    clique->nonSimilarHash = NULL; //set as NULL because we won't create and populate it yet

    return clique;
}

void Clique_Free(void* value){
    Clique* clique = (Clique*)value;

    List_Destroy(&clique->similar);
    List_Destroy(&clique->nonSimilar);
    if (clique->nonSimilarHash){
        Hash_Destroy(*(clique->nonSimilarHash));
        free(clique->nonSimilarHash);
    }

    free(clique);
}

void CliqueGroup_Init(CliqueGroup* cg, int bucketSize,unsigned int (*hashFunction)(const void*, unsigned int), bool (*cmpFunction)(void*, void*)){
    Hash_Init(&cg->hash, bucketSize, hashFunction, cmpFunction, true);
    List_Init(&cg->cliques);
    cg->finalizeNeeded = false;
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
    List_FreeValues(cg.cliques, Clique_Free);
    List_Destroy(&(cg.cliques));
}

void CliqueGroup_FreeValues(CliqueGroup cg, void (*subFree)(void*)){
    /* Free all items in every list in cliques list*/
    Node* tempNode1 = cg.cliques.head;
    while (tempNode1 != NULL){
        Clique* insideClique = (Clique*)(tempNode1->value);
        List* insideList = &insideClique->similar;
        Node* tempNode2 = insideList->head;
        while (tempNode2 != NULL){
            ItemCliquePair* icp = (ItemCliquePair*)(tempNode2->value);
            subFree(icp->item);
            tempNode2 = tempNode2->next;
        }
        tempNode1 = tempNode1->next;
    }
}

bool CliqueGroup_Update_Similar(CliqueGroup* cg, void* key1, int keySize1, void* key2, int keySize2){
    cg->finalizeNeeded = true;

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

    Clique* mergedCliques = Clique_New();

    /* NOTE: this is why we need the parent node in the ItemCliquePair(to remove the nodes from cliques list) */
    /* save old parent nodes to remove them from the cliques list later on, since they will be changed in MergeCliques*/
    Node* oldParentNode1 = icp1->cliqueParentNode;
    Node* oldParentNode2 = icp2->cliqueParentNode;
    /* save old cliques to destroy them after the merge and append is complete */
    Clique* oldClique1 = icp1->clique;
    Clique* oldClique2 = icp2->clique;

    List_Append(&cg->cliques, mergedCliques);
    CliqueGroup_MergeCliques(mergedCliques, *icp1->clique,*icp2->clique, cg->cliques.tail);


    /* remove the old parent nodes from the cliques list */
    List_RemoveNode(&cg->cliques, oldParentNode1);
    List_RemoveNode(&cg->cliques, oldParentNode2);

    /* free the old cliques */
    Clique_Free(oldClique1);
    Clique_Free(oldClique2);

    return true;
}

bool CliqueGroup_Update_NonSimilar(CliqueGroup* cg, void* key1, int keySize1, void* key2, int keySize2) {
    cg->finalizeNeeded = true;
   
    ItemCliquePair* icp1 = Hash_GetValue(cg->hash, key1, keySize1);
    if(icp1 == NULL)
        return false;
    ItemCliquePair* icp2 = Hash_GetValue(cg->hash, key2, keySize2);
    if(icp2 == NULL)
        return false;

    // We take care of duplicates after CliqueGroup_Finalize.
    List_Append(&icp1->clique->nonSimilar, icp2);
    List_Append(&icp2->clique->nonSimilar, icp1);

    return true;
}

void CliqueGroup_PrintPairs(List pairs, void (*Print)(void* value)){
    Node* currPairNode = pairs.head;
    while(currPairNode != NULL){
        Tuple* tuple = currPairNode->value;

        ItemCliquePair** icpPair = tuple->value1;

        ItemCliquePair *icpA = icpPair[0];
        ItemCliquePair *icpB = icpPair[1];

        Print(icpA->item);
        printf(" ");
        Print(icpB->item);
        printf("\n");

        currPairNode = currPairNode->next;
    }
}

List CliqueGroup_GetIdenticalPairs(CliqueGroup* cg){
    List pairs;
    List_Init(&pairs);

    Node* currCliqueNode = cg->cliques.head;
    while (currCliqueNode != NULL){
        // getting each clique.
        Clique* currClique = (Clique*)(currCliqueNode->value);
        List currCliqueList = currClique->similar;
        if (currCliqueList.size > 1){ // Only getting cliques that contain 2+ items.

            // getting each similar pair.
            Node* currItemA = currCliqueList.head;
            while (currItemA != NULL){
                Node* currItemB = currItemA->next;
                while(currItemB != NULL) {
                    ItemCliquePair *icpA = (ItemCliquePair *) (currItemA->value);
                    ItemCliquePair *icpB = (ItemCliquePair *) (currItemB->value);

                    //Alloc tuple
                    Tuple* tuple = malloc(sizeof(Tuple));
                    
                    //Alloc tuple values
                    ItemCliquePair** icpPair = malloc(2*sizeof(ItemCliquePair*));
                    bool* similarityPtr = malloc(sizeof(bool));
                    
                    //Set values
                    icpPair[0] = icpA;
                    icpPair[1] = icpB;
                    *similarityPtr = true;

                    //Set the Tuple
                    tuple->value1 = icpPair;
                    tuple->value2 = similarityPtr;

                    //Append to list
                    List_Append(&pairs, tuple);

                    currItemB = currItemB->next;
                }
                currItemA = currItemA->next;
            }

        }
        currCliqueNode = currCliqueNode->next;
    }

    return pairs;
}

List CliqueGroup_GetNonIdenticalPairs(CliqueGroup* cg){
    List pairs;
    List_Init(&pairs);

    Hash checkedCliques;
    Hash_Init(&checkedCliques, DEFAULT_HASH_SIZE, RSHash, StringCmp, true);

    Node* currCliqueNode = cg->cliques.head;
    while (currCliqueNode != NULL){
        // getting each clique.
        Clique* currClique = (Clique*)(currCliqueNode->value);
        Hash_Add(&checkedCliques, &currClique->id, sizeof(int), "-");

        // getting each similar pair.
        Node* currItemNodeA = currClique->similar.head;
        while (currItemNodeA != NULL){

            Node* currNonSimilarCliqueNode = currClique->nonSimilar.head;
            while (currNonSimilarCliqueNode != NULL){
                Clique* currNonSimilarClique = ((ItemCliquePair*)currNonSimilarCliqueNode->value)->clique;

                // This pair was added prior so we skip if the clique is found in the hash.
                if(!Hash_GetValue(checkedCliques, &currNonSimilarClique->id, sizeof(int))){

                    Node* currItemNodeB = currNonSimilarClique->similar.head;
                    while(currItemNodeB != NULL){
                        ItemCliquePair *icpA = (ItemCliquePair *) (currItemNodeA->value);
                        ItemCliquePair *icpB = (ItemCliquePair *) (currItemNodeB->value);

                        //Alloc tuple
                        Tuple* tuple = malloc(sizeof(Tuple));
                        //Alloc tuple values
                        ItemCliquePair** icpPair = malloc(2*sizeof(ItemCliquePair*));
                        bool* similarityPtr = malloc(sizeof(bool));
                        
                        //Set values
                        icpPair[0] = icpA;
                        icpPair[1] = icpB;
                        *similarityPtr = false;

                        //Set the Tuple
                        tuple->value1 = icpPair;
                        tuple->value2 = similarityPtr;
                        
                        //Append to list
                        List_Append(&pairs, tuple);

                        currItemNodeB = currItemNodeB->next;
                    }
                }

                currNonSimilarCliqueNode = currNonSimilarCliqueNode->next;
            }

            currItemNodeA = currItemNodeA->next;
        }


        currCliqueNode = currCliqueNode->next;
    }

    Hash_Destroy(checkedCliques);

    return pairs;
}

void CliqueGroup_MergeCliques(Clique* newClique, Clique clique1, Clique clique2, Node* cliqueParentNode){
    /* merges 2 cliques into one and changes all the pointers of the ItemCliquePairs to the correct ones */
    // TODO: make merging faster and call half the updates on the pointers.

    //for clique1
	Node* temp1 = clique1.similar.head;
	while(temp1 != NULL){
        ItemCliquePair* icp = (ItemCliquePair*)(temp1->value);
        icp->clique = newClique;
        icp->cliqueParentNode = cliqueParentNode;
        List_Append(&newClique->similar, icp);

		temp1 = temp1->next; //next
	}

    //for clique2
	Node* temp2 = clique2.similar.head;
	while(temp2 != NULL){
        ItemCliquePair* icp = (ItemCliquePair*)(temp2->value);
        icp->clique = newClique;
        icp->cliqueParentNode = cliqueParentNode;
        List_Append(&newClique->similar, icp);

		temp2 = temp2->next; //next
	}

	newClique->nonSimilar = List_Merge(clique1.nonSimilar, clique2.nonSimilar);
}

bool pointercmp(void* value1, void* value2){
    return (*(Clique**)value1 == *(Clique**)value2);
}

void CliqueGroup_Finalize(CliqueGroup cg){ //this should run after the CliqueGroup is updated (however many times)
    cg.finalizeNeeded = false;

    List* cliques = &cg.cliques; // list of all cliques

    //for all cliques
    Node* cliqueNode = cliques->head;
    while(cliqueNode != NULL){
        //for each clique
        Clique* clique = (Clique*)cliqueNode->value;
        Node* icpNode = clique->nonSimilar.head;
        Hash* tempHash = malloc(sizeof(Hash));
        //create hash to help with removing icps with the same cliques
        Hash_Init(tempHash, clique->similar.size * 3, cg.hash.hashFunction, pointercmp, true);
        while(icpNode != NULL){
            ItemCliquePair* icp = (ItemCliquePair*)icpNode->value;
            bool existsInHash = Hash_GetValue(*tempHash, &icp->clique, sizeof(icp->clique));

            if (!existsInHash){
                Hash_Add(tempHash, &icp->clique, sizeof(icp->clique), icp);
            }else{
                Node* icpNext = icpNode->next;
                List_RemoveNode(&clique->nonSimilar, icpNode);
                icpNode = icpNext;
                continue;
            }

            icpNode = icpNode->next; //next icp
        }
        
        if (clique->nonSimilarHash){ //if this is not NULL, finalize ran again before, this was malloc'ed
            Hash_Destroy(*(clique->nonSimilarHash));
            free(clique->nonSimilarHash);
        }
        clique->nonSimilarHash = tempHash;

        cliqueNode = cliqueNode->next; //next clique
    }
}

List CliqueGroup_GetAllItems(CliqueGroup group){
    List items;
    List_Init(&items);

    Node* currCliqueNode = group.cliques.head;
    while(currCliqueNode != NULL){
        Clique* currClique = (Clique*)currCliqueNode->value;

        Node* currItemNode = currClique->similar.head;
        while(currItemNode != NULL) {
            List_Append(&items, (ItemCliquePair*)currItemNode->value);

            currItemNode = currItemNode->next;
        }

        currCliqueNode = currCliqueNode->next;
    }

    return items;
}

bool CliqueGroup_PairIsValid(ItemCliquePair* icp1, ItemCliquePair* icp2, bool isEqual){
    if (!isEqual){
        Node* icpNode = icp1->clique->similar.head;
        while(icpNode != NULL){
            ItemCliquePair* icp = (ItemCliquePair*)icpNode->value;
            if (icp->id == icp2->id){
                return false;
            }

            icpNode = icpNode->next;
        }
    }else{
        //cannon use nonSimilarHash because we don't finilize after each insertion in retraining
        Node* icpNode = icp1->clique->nonSimilar.head;
        while(icpNode != NULL){
            ItemCliquePair* icp = (ItemCliquePair*)icpNode->value;
            if (icp->clique->id == icp2->clique->id){
                return false;
            }

            icpNode = icpNode->next;
        }
    }

    return true;
}