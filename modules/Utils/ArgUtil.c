#include <stdio.h>
#include <string.h>
#include "ArgUtil.h"

bool FindFlagIndex(char** argv, int argc, char* flag, int* index){
    //Returns the index of the flag inside argv to index argument.
    //If the flag is not found returns false.

    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i],flag) == 0){
            *index = i;
            return true;
        }
    }

    return false;
}

bool FindArgAfterFlag(char** argv, int argc, char* flag, char** value){
    //Finds the index of the flag inside argv..
    //Checks if it's the last argument , if so that means we have no more
    //and that's incorrect.When successful value is the argument after the flag.

    int index;
    if(!FindFlagIndex(argv, argc, flag, &index))
        return false;
    if(index + 1 >= argc)
        return false;

    *value = argv[index + 1];
    return true;
}

