#ifndef FOLDERUTILITIES_H
#define FOLDERUTILITIES_H

#include <stdbool.h>
#include <dirent.h>

#include "List.h"

bool GetFolderItems(char* input_dir, List* itemList);
bool IsValidItem(struct dirent* dir);

#endif
