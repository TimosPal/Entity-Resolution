#include "CSVParser.h"

#include <stdbool.h>

#include "Util.h"
#include "StringUtil.h"

bool CSV_GetLine(FILE* fp,List* values){
    char buffer[BUFFER_SIZE];
    int result = fscanf(fp, "%s\n", buffer);
    *values = StringSplit(buffer, ",");

    return (result != EOF) ? true : false;
}