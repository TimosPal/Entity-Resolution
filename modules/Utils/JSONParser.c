#include "JSONParser.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

void ValuePair_Init(ValuePair* vp){
    vp->leftVal = NULL;
    List_Init(&vp->rightVals);
}

void ValuePair_Free(void* value){ // frees value pairs
    ValuePair* vp = (ValuePair*)value;
    free(vp->leftVal);
    List_FreeValues(vp->rightVals,free);
    List_Destroy(&vp->rightVals);
    free(vp);
}

char* GetStringBetweenQuotes(FILE* fp){
    /* NOTE: fp starts after " */
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
    
    List pairs;
    List_Init(&pairs);

    char c;
    bool makeNew = true;
    bool inArray = false;
    while ((c = getc(fp)) != EOF){  // while no EOF
        if (c == '"'){
            char* str = GetStringBetweenQuotes(fp);
            
            if (makeNew){ // if a new ValuePair for the list node should be made
                ValuePair* vp = malloc(sizeof(ValuePair));
                ValuePair_Init(vp);
                vp->leftVal = str;

                List_Append(&pairs, vp);

                makeNew = false; // next loop we fill this pair
            }else{ // if we are filling the previous pair
                List* l = &((ValuePair*)(pairs.tail->value))->rightVals;
                List_Append(l,str);

                makeNew = (inArray) ? false : true; // next loop we make a new pair if we aren't inside an array.
            }
        }else if(c == '['){
            inArray = true;
        }else if(c == ']'){
            inArray = false;
            makeNew = true;
        }
    }

    fclose(fp);

    return pairs;
}