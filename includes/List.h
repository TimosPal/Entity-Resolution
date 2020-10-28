#ifndef LIST_H
#define LIST_H

typedef struct Node {
    struct Node* next;
    void* value;
}Node;

typedef struct List {
    Node* head;
    Node* tail;

    int size;
}List;

void List_Init(List* list); //Creates an empty list.

void List_AddValue(List* list, void* value, int index); //Adds a node at index.
void List_Append(List* list, void* value); //Appends a node at the end of the list.
void List_Remove(List* list, int index); //Remove a node at index.
void List_Destroy(List* list); //Destroy the list by freeing allocated nodes.
void List_FreeValues(List list,void (*freeMethod)(void*)); //Frees allocated memory for the node's values. Does NOT free the nodes.

Node* List_GetNode(List list, int index); //Get Node at index.

#endif  