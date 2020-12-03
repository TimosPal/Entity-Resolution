#ifndef ITEM_H
#define ITEM_H

#include "LinkedList.h"
#include "Hash.h"

typedef struct Item {
    char* id; // Id format : website//id_number
    List specs;
}Item;

Item* Item_Create(char* id, List specs);
List* Item_Preprocess(Item* item, Hash stopwords);
void Item_Free(void* item);
void Item_Print(void* item);

#endif