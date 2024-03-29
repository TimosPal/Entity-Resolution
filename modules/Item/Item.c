#include "Item.h"
#include "StringUtil.h"
#include "JSONParser.h"

#include <stdio.h>
#include <stdlib.h>

Item* Item_Create(char* id, List specs){
    // Only the Id should be allocated to the heap.
    Item* item = malloc(sizeof(Item));
    item->id = NewString(id);
    item->specs = specs;
    return item;
}

void Item_Free(void* item){ // frees item id and spec list
    Item* itm = (Item*)item;
    free(itm->id);
    List_FreeValues(itm->specs,ValuePair_Free);
    List_Destroy(&itm->specs);
    free(item);
}

void Item_Print(void* item){
    printf("%s", ((Item*)item)->id);
}

List* Item_Preprocess(Item* item, Hash stopwords){
    List* words = malloc(sizeof(List));
    List_Init(words);

    List specs = item->specs;
    Node* valuePairNode = specs.head;

    while(valuePairNode != NULL){
        ValuePair* valuePair = (ValuePair*)valuePairNode->value;
        List rightVals = valuePair->rightVals;

        Node* valNode = rightVals.head;
        while(valNode != NULL){
            List strings = StringPreprocess(valNode->value, stopwords);
            List_Join(words, &strings); //left join

            valNode = valNode->next;
        }

        valuePairNode = valuePairNode->next; 
    }

    //printf("ITEM ID: %s\n", item->id);
    // Node* temp = words->head;
    // while(temp!=NULL){
    //     printf("%s\n", temp->value);
    //     temp = temp->next;
    // }
    // printf("\n");
    // sleep(1);
    
    return words;
}