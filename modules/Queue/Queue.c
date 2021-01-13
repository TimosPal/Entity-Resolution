#include "Queue.h"

#include <stdlib.h>

void Queue_Init(Queue* queue){ List_Init(queue); }
void Queue_Destroy(Queue* queue){ List_Destroy(queue); }
void Queue_FreeValues(Queue* queue, void (*subFree)(void*)) { List_FreeValues(*queue, subFree);}
void Queue_Push(Queue* queue, void* value) { List_Append(queue, value); }
void* Queue_Pop(Queue* queue) {
    void* val = NULL;
    if(queue->head){
        val = queue->head->value;
        List_Remove(queue, 0);
    }

    return val;
}