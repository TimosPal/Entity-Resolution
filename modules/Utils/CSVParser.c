#include "CSVParser.h"

#include <stdbool.h>

#include "Util.h"
#include "StringUtil.h"

bool CSV_GetLine(FILE* fp,List* values){
    char buffer[BUFFER_SIZE];
    char result = fscanf(fp, "%s\n", buffer);
    
    if (result == EOF){
        return false;
    }
    
    *values = StringSplit(buffer, ",");

    return true;
}