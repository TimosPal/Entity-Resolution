#include "acutest.h"

#include "StringUtil.h"

void StringUtil_Test_StringToInt(){
    char* numStr = "153";
    int realNum;
    StringToInt(numStr, &realNum);
    TEST_ASSERT(realNum == 153);
}

void StringUtil_Test_StringSplit(){
    char str[] = "1,2,3,4,5,6";
    List l = StringSplit(str,",");
    TEST_ASSERT(*(char*)l.head->value == '1');
    TEST_ASSERT(*(char*)l.head->value == '1');
    TEST_ASSERT(*(char*)l.head->next->value == '2');
    TEST_ASSERT(*(char*)l.head->next->next->value == '3');
    TEST_ASSERT(*(char*)l.head->next->next->next->value == '4');
    List_FreeValues(l,free);
    List_Destroy(&l);
}

TEST_LIST = {
        { "StringUtil_Test_StringToInt", StringUtil_Test_StringToInt },
        { "StringUtil_Test_StringSplit", StringUtil_Test_StringSplit },
        { NULL, NULL }
};