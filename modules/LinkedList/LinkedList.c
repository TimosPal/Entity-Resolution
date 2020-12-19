#include "LinkedList.h"

#include <stdio.h>
#include <stdlib.h>

void List_Init(List* list) {
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

void List_Destroy(List* list) {
	while (list->head != NULL){
		List_Remove(list, 0);
	}
}

void List_Free(void* value){
	List* list = (List*)value;
	List_Destroy(list);
	free(list);
}

void List_AddValue(List* list, void* value, int index) {
	Node* newNode = malloc(sizeof(Node));
	newNode->value = value;
	list->size++;

	if (index == 0) {
		if (list->head == NULL) {
            list->tail = newNode;
        } else {
            list->head->prev = newNode;
        }

		newNode->prev = NULL;
		newNode->next = list->head;
		list->head = newNode;
	} else {
		Node* temp = list->head;
		for (int i = 0; i < index - 1; i++)
			temp = temp->next;

		newNode->prev = temp;
		newNode->next = temp->next;
		temp->next = newNode;

		if (newNode->next == NULL)
			list->tail = newNode;
	}
}

void List_Append(List* list, void* value) {
	Node* newNode = malloc(sizeof(Node));
	newNode->value = value;
	newNode->next = NULL;

	if(list->head == NULL){
        list->head = newNode;
        newNode->prev = NULL;
    }else{
        list->tail->next = newNode;
        newNode->prev = list->tail;
    }
	list->tail = newNode;
	list->size++;
}

bool List_Remove(List* list, int index) {
	if ( list->head == NULL ){
		return false;
	}
	Node* temp = list->head;
	(list->size)--;

	if (index == 0) {
		list->head = list->head->next;
		free(temp);

		if (list->head == NULL) {
            list->tail = NULL;
        }else{
		    list->head->prev = NULL;
		}
	} else {
		for (int i = 0; i < index - 1; i++)
			temp = temp->next;

		Node* old = temp->next;
		temp->next = old->next;
		free(old);

		if (temp->next == NULL) {
			list->tail = temp;
		}else{
		    temp->next->prev = temp;
		}
	}
	

	return true;
}

bool List_RemoveNode(List* list, Node* node){
    if(node->prev == NULL){ // we remove the first node.
        list->head = node->next;
    }else{
        node->prev->next = node->next;
    }

    if(node->next == NULL){ // we remove the last node.
        list->tail = node->prev;
    }else{
        node->next->prev = node->prev;
    }

    free(node);

	(list->size)--;

	return true;
}

Node* List_GetNode(List list, int index) {
	Node* temp = list.head;
	for (int i = 0; i < index; i++)
		temp = temp->next;

	return temp;
}

void* List_GetValue(List list, int index) {
	Node* temp = list.head;
	for (int i = 0; i < index; i++)
		temp = temp->next;
	
	return temp->value;
}

void List_FreeValues(List list, void (*subFree)(void*)){
	Node* currNode = list.head;
	while(currNode != NULL){
		subFree(currNode->value);

		currNode = currNode->next;
	}
}

bool List_ValueExists(List list, void* value){
	Node* temp = list.head;
	while (temp != NULL){
		if (temp->value == value){
			return true;
		}
		temp = temp->next;
	}
	return false;
}

List List_Merge(List list1, List list2){
	List newList;
	List_Init(&newList);

	Node* temp1 = list1.head;
	while(temp1 != NULL){
		if (!List_ValueExists(newList, temp1->value)){
			List_Append(&newList, temp1->value);
		}
		temp1 = temp1->next;
	}

	Node* temp2 = list2.head;
	while(temp2 != NULL){
		if (!List_ValueExists(newList, temp2->value)){
			List_Append(&newList, temp2->value);
		}
		temp2 = temp2->next;
	}

	return newList;
}

void** List_ToArray(List list){
	Node* currNode = list.head;
    void** array = malloc(list.size * sizeof(void*));
    int iter = 0;

    while(currNode != NULL){
        array[iter] = currNode->value;

		currNode = currNode->next;
        iter++;
    }

	return array;
}

void List_Join(List* list1, List* list2){
	if (list1->size == 0){
		*list1 = *list2;
	}else{
		list1->tail->next = list2->head;
		if(list2->size != 0){
			list2->head->prev = list1->tail;
		    list1->tail = list2->tail;
		}
		list1->size += list2->size;
	}
		
    list2->size = 0;
    list2->tail = NULL;
    list2->head = NULL;
}

/* List 1 will be splitted , percentage of list1 in decimal*/
bool List_Split(List* list1, List* list2,  double percentage){
	List_Init(list2); //just to be sure list 2 has been initialized with 0s
	
	int index = list1->size * percentage;

	if (index < 2){
		return false;
	}

	Node* splitNode = List_GetNode(*list1, index); //splitNode will be in list2

	Node* list1_tail = splitNode->prev;

	//for list2
	list2->head = splitNode;
	list2->head->prev = NULL;
	list2->tail = list1->tail;
	list2->size = list1->size - index;

	//for list1
	list1->tail = list1_tail;
	list1->tail->next = NULL;
	list1->size = index;


	return true;
}