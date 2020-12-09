#ifndef CUSTOM_STRING_FUNCTIONS_H
#define CUSTOM_STRING_FUNCTIONS_H

#include <stdbool.h>
#include <string.h>

#include "LinkedList.h"
#include "Hash.h"

void StringReplaceChar(char* str, char old, char new);
void StringLineCleanup(char* str);

char* NewString(char* str);
char* GetFileNameFromPath(char* str);

bool StringToInt(char* str,int* value);
bool StringCmp(void* value1,void* value2);
bool StringIsEmptyLine(char* str);

List StringSplit(char* str, char* token);
List StringPreprocess(char* str, Hash stopwords);

unsigned int StringHash(void* str);

void RemoveFileExtension(char* str);

#endif
