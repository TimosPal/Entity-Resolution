#include "LinkedList.h"
#include "acutest.h"
#include <stdbool.h>

void Test_init(){
    List list;
    List_Init(&list);

    TEST_ASSERT(list.head == NULL);
    TEST_ASSERT(list.tail == NULL);
    TEST_ASSERT(list.size == 0);
}

void Test_insert(){
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

void Test_remove(){
    List list;
    List_Init(&list);

    int size = 100;
    int* array = malloc(size * sizeof(int));

    for(int i = 0; i < size; i++){
        array[i] = i; // array[i] gets value of i
        List_Append(&list, array+i); // append a node to list with value i (malloc'd above)
    }

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

    free(array);
}




TEST_LIST = {
    { "LinkedList_test_init",   Test_init },
    { "LinkedList_test_insert", Test_insert},
    { "LinkedList_test_remove", Test_remove},
    { NULL, NULL }
};