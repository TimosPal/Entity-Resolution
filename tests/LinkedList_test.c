#include "LinkedList.h"
#include "acutest.h"

void ListTest1(){
    int i = 5;
    TEST_CHECK(i == 5);
}


TEST_LIST = {
    { "test1", ListTest1 },
    { NULL, NULL }
};