#include "../../includes/List.h"

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

void List_Remove(List* list, int index) {
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
}

Node* List_GetNode(List list, int index) {
	Node* temp = list.head;
	for (int i = 0; i < index; i++)
		temp = temp->next;

	return temp;
}

void List_FreeValues(List list,void (*subFree)(void*)){
	Node* currNode = list.head;
	while(currNode != NULL){
		subFree(currNode->value);
		currNode = currNode->next;
	}
}

void List_DefaultFreeMethod(void* value){
	free(value);
}