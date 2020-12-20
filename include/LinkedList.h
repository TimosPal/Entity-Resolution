#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct Node {
    struct Node* next;
    struct Node* prev;
    void* value;
}Node;

typedef struct List {
    Node* head;
    Node* tail;

    int size;
}List;

void List_Init(List* list); //Creates an empty list.

bool List_Remove(List* list, int index); //Remove a node at index.
bool List_RemoveNode(List* list, Node* node); //Remove the specific node
bool List_ValueExists(List list, void* value);
bool List_Split(List* list1, List* list2,  double percentage); //splits a list into two lists based on percentage
void List_AddValue(List* list, void* value, int index); //Adds a node at index.
void List_Append(List* list, void* value); //Appends a node at the end of the list.
void List_Destroy(List* list); //Destroy the list by freeing allocated nodes.
void List_Free(void* value);
void List_FreeValues(List list,void (*freeMethod)(void*)); //Frees allocated memory for the node's values. Does NOT free the nodes.
void List_Join(List* list1, List* list2);
void List_Shuffle(List* list);
void** List_ToArray(List list);

List List_Merge(List list1, List list2); // Merges 2 existing lists into a new list and returns it

Node* List_GetNode(List list, int index); //Get Node at index.
void* List_GetValue(List list, int index); //Get value of Node at index.

#endif  