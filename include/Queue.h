#ifndef QUEUE_H
#define QUEUE_H

#include "LinkedList.h"

typedef List Queue;

void Queue_Init(Queue* queue);
void Queue_Destroy(Queue* queue);
void Queue_FreeValues(Queue* queue, void (*subFree)(void*));
void Queue_Push(Queue* queue, void* value);
void* Queue_Pop(Queue* queue);

#endif