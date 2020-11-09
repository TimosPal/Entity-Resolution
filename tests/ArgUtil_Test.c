#include "acutest.h"

#include "ArgUtil.h"

void ArgUtil_test_FindFlagIndex(){
    char* argv[] = { "prog" , "nothing" ,"-test" , "value"};
    int index = -1;
    int argc = sizeof(argv) / sizeof(char*);
    FindFlagIndex(argv,argc,"-test",&index);
    TEST_ASSERT(index == 2);
    TEST_ASSERT(FindFlagIndex(argv,argc,"not_found",&index) == false);
}

void ArgUtil_test_FindArgAfterFlag(){
    char* argv[] = { "prog" , "nothing" ,"-test" , "value" , "-f"};
    int argc = sizeof(argv) / sizeof(char*);
    char* val = "";
    FindArgAfterFlag(argv,argc,"-test",&val);
    TEST_ASSERT(strcmp(val,"value") == 0);
    TEST_ASSERT(FindArgAfterFlag(argv,argc,"-f",&val) == false);
}

TEST_LIST = {
        { "ArgUtil_test_FindFlagIndex",    ArgUtil_test_FindFlagIndex },
        { "ArgUtil_test_FindArgAfterFlag", ArgUtil_test_FindArgAfterFlag },
        { NULL, NULL }
};