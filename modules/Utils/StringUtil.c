#include "StringUtil.h"

#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

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

bool StringToInt(char* str, int* value){
    errno = 0;
    char* endPtr;

    *value = (int)strtol(str, &endPtr, 10);
    return (errno == 0 && *endPtr == '\0');
}

bool StringToDouble(char* str, double* value){
    errno = 0;
    char* endPtr;

    *value = (double)strtod(str, &endPtr);
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
        str[i] = tolower(str[i]);
        i++;
    }

    return str;
}

void RemoveUnicode(char* str){
    // Checks if str has a unicode substring , only returns the first found.

    int index = 0;
    int unicodesCounter = 0;
    bool hasFoundSlash = false;
    bool hasFoundU = false;
    while(str[index] != '\0'){
        if(!hasFoundSlash) { // Haven't found a slash yet
            if (str[index] == '\\') {
                hasFoundSlash = true;
            }
        }else{
            if(!hasFoundU) { // Have found slash not U
                if (str[index] == 'u') {
                    hasFoundU = true;
                }else{
                    hasFoundSlash = false;
                }
            }else{ // Have found slash and u
                if (isxdigit(str[index])) {
                    unicodesCounter++;
                    if (unicodesCounter == 4) {
                        // Replace with spaces.
                        for (int i = 0; i < 6; ++i) {
                            str[index - i] = ' ';
                        }

                        hasFoundSlash = false;
                        hasFoundU = false;
                        unicodesCounter = 0;
                    }
                } else {
                    hasFoundSlash = false;
                    hasFoundU = false;
                }
            }
        }

        index++;
    }
}

void RemoveNewLines(char* str){
    int index = 0;
    bool hasFoundSlash = false;
    while(str[index] != '\0'){
        if(!hasFoundSlash) { // Haven't found a slash yet
            if (str[index] == '\\') {
                hasFoundSlash = true;
            }
        }else{
            if(str[index] == 'n'){
                str[index] = ' ';
                str[index-1] = ' ';
                hasFoundSlash = false;
            }
        }

        index++;
    }
}

void RemovePunctuation(char* str){
    int index = 0;
    while(str[index] != '\0'){
        if(ispunct(str[index]) && str[index] != '.') {
            str[index] = ' ';
        }

        index++;
    }
}

void RemoveDot(char* str){
    int index = 0;
    while(str[index] != '\0'){
        if(str[index] == '.') {
            if (index - 1 >= 0 && !isdigit(str[index - 1])){
                str[index] = ' ';
            }
        }

        index++;
    }
}

void RemoveBigNumber(char* str){
    /* Remove continues digits that are more than 5 */
    int index = 0;
    int digits = 0;
    int counter = 0;
    bool hasFinished;

    do{
        hasFinished = str[index] == '\0';

        if(isdigit(str[index])){ // Increment digit and char counters.
            digits++;
            counter++;
        }else if(str[index] == ' ' || hasFinished){ // If we found a space (word ended) or string ended.
            if(digits == counter && digits > 5) { // If more than 5 digits and all are numbers.
                for (int i = 0; i < counter; ++i) { // Remove big number.
                    str[index - (i + 1)] = ' '; // Skip \0 or space
                }
            }

            counter = 0;
            digits = 0;
        }else{
            counter++;
        }

        index++;
    }while(!hasFinished);
}

void RemoveWhiteSpace(char* str){
    int index = 0;
    while(str[index] != '\0'){
        if((isspace(str[index])) || !isprint(str[index]) || iscntrl(str[index])) {
            str[index] = ' ';
        }

        index++;
    }
}

List StringPreprocess(char* str, Hash stopwords){
    char* newStr = NewString(str);
    List words = StringSplit(newStr, " ");
    free(newStr);

    Node* temp = words.head;
    //preprocess the specific words
    while(temp != NULL){
        StringToLower(temp->value);
        RemoveUnicode(temp->value);
        RemoveNewLines(temp->value);
        RemovePunctuation(temp->value);
        RemoveWhiteSpace(temp->value);
        RemoveDot(temp->value);
        RemoveBigNumber(temp->value);

        // If after the proccessig we found sub words , we add them in the list.
        List newWords = StringSplit(temp->value, " ");
        if(newWords.head == NULL){ // temp->value was an empty string.
            Node* nextNode = temp->next;
            free(temp->value);
            List_RemoveNode(&words, temp);
            temp = nextNode;
            continue;
        }

        free(temp->value);
        temp->value = newWords.head->value;
        List_Remove(&newWords,0);
        List_Join(&words,&newWords);

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