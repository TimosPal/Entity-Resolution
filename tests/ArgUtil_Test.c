#include "acutest.h"

#include "ArgUtil.h"

void ArgUtil_Test_FindFlagIndex(){
    char* argv[] = { "prog" , "nothing" ,"-test" , "value"};
    int index = -1;
    int argc = sizeof(argv) / sizeof(char*);
    FindFlagIndex(argv,argc,"-test",&index);
    TEST_ASSERT(index == 2);
    TEST_ASSERT(FindFlagIndex(argv,argc,"not_found",&index) == false);
}

void ArgUtil_Test_FindArgAfterFlag(){
    char* argv[] = { "prog" , "nothing" ,"-test" , "value" , "-f"};
    int argc = sizeof(argv) / sizeof(char*);
    char* val = "";
    FindArgAfterFlag(argv,argc,"-test",&val);
    TEST_ASSERT(strcmp(val,"value") == 0);
    TEST_ASSERT(FindArgAfterFlag(argv,argc,"-f",&val) == false);
}

TEST_LIST = {
        { "ArgUtil_Test_FindFlagIndex",    ArgUtil_Test_FindFlagIndex },
        { "ArgUtil_Test_FindArgAfterFlag", ArgUtil_Test_FindArgAfterFlag },
        { NULL, NULL }
};