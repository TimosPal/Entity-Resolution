#ifndef ITEM_H
#define ITEM_H

#include "LinkedList.h"

typedef struct Item {
    char* id; // Id format : website//id_number
    List* specs;
}Item;

Item* Item_Create(char* id, List* specs);

#endif