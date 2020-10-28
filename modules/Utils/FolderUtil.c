#include "FolderUtil.h"

#include <string.h>
#include <stdio.h>

#include "Util.h"

bool GetFolderItems(char* input_dir, List* itemList){
    // Returns a list of the items contained inside the provided path.

    //Open the folder containing the data.
    DIR* directory = opendir(input_dir);
    if(directory == NULL) //Directory could not be opened
        return false;

    List_Init(itemList);

    //Access each item.
    struct dirent* dir;
    char buffer[BUFFER_SIZE];
    while((dir = readdir(directory))){
        if(!IsValidItem(dir)) continue;

        sprintf(buffer,"%s",dir->d_name);
        char* itemName = NewString(buffer);
        List_Append(itemList, itemName);
    }

    if(closedir(directory) == -1) //Directory could not be closed"
        return false;

    return true;
}

bool IsValidItem(struct dirent* dir) {
    if (dir->d_ino == 0) //Skip delete inodes.
        return false;
    if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) //Skip . ..
        return false;
    return true;
}