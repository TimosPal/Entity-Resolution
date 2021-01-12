#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>

#include "LinkedList.h"

typedef List Queue;

void Queue_Init(Queue* queue){ List_Init(queue); }
void Queue_Destroy(Queue* queue){ List_Destroy(queue); }
void Queue_Pop(Queue* queue) { List_Remove(queue, 0); }
void Queue_Add(Queue* queue, void* value) { List_Append(queue, value); }

#endif