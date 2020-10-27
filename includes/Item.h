#ifndef ITEM_H
#define ITEM_H

#include "List.h"

typedef struct Item {
    char* id;
    List* specs;
}Item;

#endif