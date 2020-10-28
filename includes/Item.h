#ifndef ITEM_H
#define ITEM_H

#include "List.h"

typedef struct Item {
    char* id; // Id format : website//id_number
    List* specs;
}Item;

#endif