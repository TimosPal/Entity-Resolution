#include "Item.h"

#include <stdlib.h>

#include "StringUtil.h"

Item* Item_Create(char* id, List specs){
    // Only the Id should be allocated to the heap.
    Item* item = malloc(sizeof(Item));
    item->id = NewString(id);
    item->specs = specs;
    return item;
}