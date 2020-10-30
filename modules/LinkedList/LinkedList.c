#include "LinkedList.h"

#include <stdio.h>
#include <stdlib.h>

void List_Init(List* list) {
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

void List_Destroy(List* list) {
	while (list->head != NULL)
		List_Remove(list, 0);
}

void List_AddValue(List* list, void* value, int index) {
	Node* newNode = malloc(sizeof(Node));
	newNode->value = value;
	list->size++;

	if (index == 0) {
		if (list->head == NULL)
			list->tail = newNode;

		newNode->next = list->head;
		list->head = newNode;
	} else {
		Node* temp = list->head;
		for (int i = 0; i < index - 1; i++)
			temp = temp->next;

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

	if (list->head == NULL)
		list->head = newNode;
	else
		list->tail->next = newNode;
	list->tail = newNode;
	list->size++;
}

bool List_Remove(List* list, int index) {
	if ( list->head == NULL ){
		return false;;
	}
	Node* temp = list->head;
	list->size--;

	if (index == 0) {
		list->head = list->head->next;
		free(temp);

		if (list->head == NULL)
			list->tail = NULL;
	} else {
		for (int i = 0; i < index - 1; i++)
			temp = temp->next;

		Node* old = temp->next;
		temp->next = old->next;
		free(old);

		if (temp->next == NULL) {
			list->tail = temp;
		}
	}
	
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

List List_Merge(List list1, List list2){
	List newList;
	List_Init(&newList);

	Node* temp1 = list1.head;
	while(temp1 != NULL){
		List_Append(&newList, temp1->value);
		temp1 = temp1->next;
	}

	Node* temp2 = list2.head;
	while(temp2 != NULL){
		List_Append(&newList, temp2->value);
		temp2 = temp2->next;
	}

	return newList;
}