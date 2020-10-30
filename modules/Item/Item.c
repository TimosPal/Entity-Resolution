#include "Item.h"

#include <stdlib.h>
#include "StringUtil.h"

#include "JsonParser.h"

Item* Item_Create(char* id, List specs){
    // Only the Id should be allocated to the heap.
    Item* item = malloc(sizeof(Item));
    item->id = NewString(id);
    item->specs = specs;
    return item;
}

void Item_Free(void* item){
    Item* itm = (Item*)item;
    free(itm->id);
    List_FreeValues(itm->specs,ValuePair_Free);
    List_Destroy(&itm->specs);
}