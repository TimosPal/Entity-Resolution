#include "StringUtil.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void StringReplaceChar(char* str, char old, char new){
    char *charPtr = strchr(str, old);
    if (charPtr) *charPtr = new;
}

List StringSplit(char* str, char* token){
    List temp;
    List_Init(&temp);
    char* currWord;
    currWord = strtok(str, token);
    while(currWord != NULL){
        List_Append(&temp, NewString(currWord));
        currWord = strtok(NULL, token);
    }

    return temp;
}

bool StringToInt(char* str,int* value){
    errno = 0;
    char* endPtr;

    *value = (int)strtol(str,&endPtr,10);
    return (errno == 0 && *endPtr == '\0');
}

char* NewString(char* str){
    char* newStr = malloc((strlen(str) + 1)*sizeof(char));
    strcpy(newStr,str);
    return newStr;
}

bool StringCmp(void* value1,void* value2){
    return strcmp((char*)value1,(char*)value2) == 0;
}

unsigned int StringHash(void* str){
    unsigned int sum = 0;
    char* temp = (char*)str;
    while(*temp != '\0'){
        sum = *temp + 31*sum;
        temp++;
    }
    return sum;
}

void StringLineCleanup(char* str){
    //Removes \r \n.
    StringReplaceChar(str,'\n',0);
    StringReplaceChar(str,'\r',0);
}

bool StringIsEmptyLine(char* str){
    //For Skipping blank lines.
    return str[0] == '\n' || str[0] == '\r';
}

char* GetFileNameFromPath(char* str){
    // NOTE: stringSplit alters the str buffer
    // so the str memory is not to be freed from within the list.

    List strings = StringSplit(str,"/");
    char* fileName = NewString(strings.tail->value);
    List_Destroy(&strings);
    return fileName;
}

void RemoveFileExtension(char* str){
    int length = strlen(str);

    char* temp = str + length - 1;
    while (*temp != '.'){
        temp--;
    }

    *temp = '\0';

    return;
}

char* StringToLower(char* str){
    int i = 0;
    while(str[i] != '\0'){
        if (str[i] >= 'A' && str[i] <= 'Z'){
            str[i] += 'a' - 'A';
        }

        i++;
    }

    return str;
}

List StringPreprocess(char* str, Hash stopwords){
    char* newStr = NewString(str);
    List words = StringSplit(newStr, " ");
    free(newStr);

    Node* temp = words.head;
    //preprocess the specific words
    while(temp != NULL){
        //remove stopwords
        StringToLower((char*)temp->value);
        //printf("%s\n", (char*)temp->value);
        
        //delete words that are in stopwords
        if (Hash_GetValue(stopwords, temp->value, strlen((char*)(temp->value))+1)){
            Node* nextNode = temp->next;
            free(temp->value);
            List_RemoveNode(&words, temp);
            temp = nextNode;
            continue;
        }
        
        temp = temp->next;
    }


    return words;
}