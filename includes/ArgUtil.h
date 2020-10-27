#ifndef ARGUTIL_H
#define ARGUTIL_H

#include <stdbool.h>

bool FindFlagIndex(char** argv, int argc, char* flag, int* index);
bool FindArgAfterFlag(char** argv, int argc, char* flag, char** value);

#endif