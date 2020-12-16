#include "Tuple.h"

#include <string.h>
#include <stdlib.h>


void Tuple_Init(Tuple* tuple, void* value1, unsigned int size1, void* value2, unsigned int size2){
    void* a = malloc(size1);
    void* b = malloc(size2);

    memcpy(a, value1, size1);
    memcpy(b, value2, size2);

    tuple->value1 = a;
    tuple->value2 = b;
}

void Tuple_Free(void* value){
    Tuple* tuple = (Tuple*)value;

    Tuple_FreeValues(*tuple, free);
    free(tuple);
}

void Tuple_FreeValues(Tuple tuple, void (*free_func)(void*)){
    free_func(tuple.value1);
    free_func(tuple.value2);
}

void Tuple_TuplesToLists(List tuples, List* val1s, List* val2s){
    List_Init(val1s);
    List_Init(val2s);

    Node* currNode = tuples.head;
    while(currNode != NULL){
        Tuple* currTuple = currNode->value;

        List_Append(val1s, currTuple->value1);
        List_Append(val2s, currTuple->value2);

        currNode = currNode->next;
    }
}