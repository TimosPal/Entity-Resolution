#include "LinkedList.h"
#include "acutest.h"
#include <stdbool.h>

void LinkedList_Test_Init(){
    List list;
    List_Init(&list);

    TEST_ASSERT(list.head == NULL);
    TEST_ASSERT(list.tail == NULL);
    TEST_ASSERT(list.size == 0);
}

void LinkedList_Test_Insert(){
    List list;
    List_Init(&list);

    int size = 100;
    int* array = malloc(size * sizeof(int));

    for(int i = 0; i < size; i++){
        array[i] = i; // array[i] gets value of i
        List_Append(&list, array+i); // append a node to list with value i (malloc'd above)

        /* size must have been increased by 1 */
        TEST_ASSERT(list.size == i + 1);

        /* tail value must match array+i */
        TEST_ASSERT(list.tail->value == array+i);
    }

    TEST_ASSERT(list.head->prev == NULL); // first node prev is null.
    TEST_ASSERT(list.head->next->prev == list.head); // first two nodes are connected correctly.
    TEST_ASSERT(list.head->next->next->prev == list.head->next); // second and third nodes are connected correctly.

    for(int i = 0; i < size; i++){
        /* values must match */
        TEST_ASSERT(*(int*)List_GetValue(list, i) == i);
    }

    /* insert at index 50 */
    int temp = 420;
    int index = 50;

    List_AddValue(&list, &temp, index);
    /* values must be equal */
    TEST_ASSERT(*(int*)List_GetValue(list, index) == temp);

    List_Destroy(&list);
    free(array);
}

void LinkedList_Test_ValueExist(){
    List list;
    List_Init(&list);

    int a = 10;
    int b = 20;
    int c = 30;

    List_Append(&list, &a);
    List_Append(&list,&b);

    TEST_ASSERT(List_ValueExists(list, &a) == true);
    TEST_ASSERT(List_ValueExists(list, &b) == true);
    TEST_ASSERT(List_ValueExists(list, &c) == false);

    List_Destroy(&list);
}

void LinkedList_Test_Merge(){
    List list1;
    List_Init(&list1);
    List list2;
    List_Init(&list2);

    int a = 10;
    int b = 20;
    List_Append(&list1, &a);
    List_Append(&list2,&b);

    List list3 = List_Merge(list1,list2);
    TEST_ASSERT(list3.size == 2);
    TEST_ASSERT(*(int*)list3.head->value == 10);
    TEST_ASSERT(*(int*)list3.tail->value == 20);

    List_Destroy(&list1);
    List_Destroy(&list2);
    List_Destroy(&list3);
}

void LinkedList_Test_Remove(){
    List list;
    List_Init(&list);

    int size = 100;
    int* array = malloc(size * sizeof(int));

    for(int i = 0; i < size; i++){
        array[i] = i; // array[i] gets value of i
        List_Append(&list, array+i); // append a node to list with value i (malloc'd above)
    }

    List_AddValue(&list, &array[0],0); // append a node to list with value i (malloc'd above)
    List_Remove(&list, 0);
    TEST_ASSERT(list.head->prev == NULL); // head prev should still be null.

    for(int i = 0; i < size; i++){
        /* Head value should be i */
        TEST_ASSERT(*(int*)list.head->value == i);
        /* Tail value should stay the same */
        TEST_ASSERT(*(int*)list.tail->value == size - 1);

        /* Remove first node */
        List_Remove(&list, 0);

        /* Test that size was decreased by 1 */
        TEST_ASSERT(list.size == size - i - 1);
    }

    /* We fill up the list again to check for tail removal */
    for(int i = 0; i < size; i++){
        array[i] = i; // array[i] gets value of i
        List_Append(&list, array+i); // append a node to list with value i (malloc'd above)
    }

    for(int i = 0; i < size; i++){
        /* Head value should be stay the same */
        TEST_ASSERT(*(int*)list.head->value == 0);
        /* Tail value should be decreased by 1 each time */
        TEST_ASSERT(*(int*)list.tail->value == size - i - 1);

        /* Remove last node */
        List_Remove(&list, list.size - 1);

        /* Test that size was decreased by 1 */
        TEST_ASSERT(list.size == size - i - 1);
    }

    /* We fill up the list again to check for intermediate removal */
    for(int i = 0; i < size; i++){
        array[i] = i; // array[i] gets value of i
        List_Append(&list, array+i); // append a node to list with value i (malloc'd above)
    }

    int index = 50;
    int nextNodeValue = *(int*)List_GetValue(list, index + 1);

    List_Remove(&list, index);
    /* Test that size was decreased by 1 */
    TEST_ASSERT(list.size == size - 1);
    /* Check that value of index-th node is now the next node's value */
    TEST_ASSERT(*(int*)List_GetValue(list, index) == nextNodeValue);

    List_Destroy(&list);

    /* edge cases */
    List_Init(&list);

    /* remove head */
    bool flag = List_Remove(&list, 0);
    TEST_ASSERT(list.size == 0);
    TEST_ASSERT(flag == false);
    

    /* remove another one */
    flag = List_Remove(&list, 5);
    TEST_ASSERT(list.size == 0);
    TEST_ASSERT(flag == false);


    for(int i = 0; i < 3; i++) {
        List_Append(&list,NULL);
    }
    Node* a = list.head;
    Node* b = a->next;
    Node* c = b->next;
    List_RemoveNode(&list,b);
    TEST_ASSERT(list.tail == c);
    TEST_ASSERT(c->prev == a);

    List_RemoveNode(&list,c);
    TEST_ASSERT(a->next == NULL);
    TEST_ASSERT(list.tail == a);

    List_RemoveNode(&list,a);
    TEST_ASSERT(list.head == NULL);
    TEST_ASSERT(list.tail == NULL);

    List_Destroy(&list);
    free(array);
}

void LinkedList_Test_Split(){
    List list1;
    List_Init(&list1);

    int value1 = 1, value2 = 2, value3 = 3;
    List_Append(&list1, &value1);
    List_Append(&list1, &value2);
    List_Append(&list1, &value3);
    
    List list2;
    List_Split(&list1, &list2, 0.8); // here list1 should have 2 items after the split

    TEST_ASSERT(list1.size == 2);
    TEST_ASSERT(list2.size == 1);

    //for list1
    TEST_ASSERT(list1.head->value == &value1);
    TEST_ASSERT(list1.head->next->value == &value2);
    TEST_ASSERT(list1.head->next->next == NULL);
    TEST_ASSERT(list1.head->prev == NULL);
    TEST_ASSERT(list1.tail->next == NULL);
    
    //for list2
    TEST_ASSERT(list2.size == 1);
    TEST_ASSERT(list2.head->value == &value3);
    TEST_ASSERT(list2.head->next == NULL);
    TEST_ASSERT(list2.head->prev == NULL);
    TEST_ASSERT(list2.tail == list2.head);
    TEST_ASSERT(list2.tail->next == NULL);

    List_Destroy(&list1);
    List_Destroy(&list2);
    
}

void LinkedList_Test_Shuffle(){
    List list;
    List_Init(&list);

    int zero = 0, one = 1;
    for (int i = 0; i < 50; i++){
        List_Append(&list, &one);
    }

    for (int i = 0; i < 200; i++){
        List_Append(&list, &zero);
    }

    List_Shuffle(&list);

    Node* node = list.head;
    while(node != NULL){
        //printf("%d\n", *(int*)node->value);

        node = node->next;
    }

    List_Destroy(&list);
}

TEST_LIST = {
    { "LinkedList_Test_init",        LinkedList_Test_Init },
    { "LinkedList_Test_insert",      LinkedList_Test_Insert },
    { "LinkedList_Test_remove",      LinkedList_Test_Remove },
    { "LinkedList_Test_value_exist", LinkedList_Test_ValueExist },
    { "LinkedList_Test_merge",       LinkedList_Test_Merge },
    { "LinkedList_Test_Split", LinkedList_Test_Split },
    { "LinkedList_Test_Shuffle", LinkedList_Test_Shuffle },
    { NULL, NULL }
};