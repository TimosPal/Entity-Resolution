#include "JsonParser.h"

#include <stdbool.h>
#include <stdlib.h>

char* GetStringBetweenQuotes(FILE* fp){
    /* NOTE: fp starts after " */
    //FILE* savedfp = fp; // save starting position of fp for the copy into the buffer later on
    
    bool hasReadBS = false; // has read backslash (to ignore ")
    
    int stringLength = 0;
    char c;
    while ((c = getc(fp)) != '"' || hasReadBS){
        if (hasReadBS)
            hasReadBS = false;
        if (c == '\\') 
            hasReadBS = true;
        stringLength++;
    }

    char* str = malloc((stringLength+1) * sizeof(char));
    
    fseek(fp, -(stringLength+1), SEEK_CUR);
    fread(str, stringLength, 1, fp); // read between the " into a buffer
    fseek(fp, 1, SEEK_CUR); //get last "
    
    str[stringLength] = '\0';

    return str;
}

List GetJsonPairs(char* filePath){
    /* NOTE: Assume json format is correct */
    /* Open file */
    FILE* fp = fopen(filePath, "r");
    
    List list;
    List_Init(&list);

    char c;
    bool makeNew = true;
    while ((c = getc(fp)) != EOF){  // while no EOF
        if (c == '"'){
            char* str = GetStringBetweenQuotes(fp);
            
            if (makeNew){ // if a new ValuePair for the list node should be made
                ValuePair* vp = malloc(sizeof(ValuePair));
                vp->leftVal = str;

                List_Append(&list, vp);

                makeNew = false; // next loop we fill this pair
            }else{ // if we are filling the previous pair
                ((ValuePair*)(list.tail->value))->rightVal = str;

                makeNew = true; // next loop we make a new pair
            }
        }
    }

    fclose(fp);

    return list;
}