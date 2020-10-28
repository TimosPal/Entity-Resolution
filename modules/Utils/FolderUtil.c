#include "FolderUtil.h"

#include <string.h>
#include <stdio.h>

#include "Util.h"

bool GetFolderItems(char* input_dir, List* folderItemsList){
    //Open the folder containing the data.
    DIR* directory = opendir(input_dir);
    if(directory == NULL){
        perror("Directory could not be opened");
        return false;
    }

    List_Init(folderItemsList);

    //Access each sub folder.
    struct dirent* dir;
    char buffer[BUFFER_SIZE];
    while((dir = readdir(directory))){
        if(!IsFolder(dir)) continue;

        sprintf(buffer,"%s/%s",input_dir,dir->d_name);
        char* itemName = NewString(buffer);
        List_Append(folderItemsList,itemName);
    }

    if(closedir(directory) == -1){
        perror("Directory could not be closed");
        return false;
    }

    return true;
}

bool IsFolder(struct dirent* dir) {
    if (dir->d_ino == 0) //Skip delete inodes.
        return false;
    if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) //Skip . ..
        return false;
    return true;
}