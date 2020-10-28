#ifndef FOLDERUTILITIES_H
#define FOLDERUTILITIES_H

#include <stdbool.h>
#include <dirent.h>

#include "List.h"
#include "Utilities.h"

bool SendFolderGroup(List folders, int fd, int chunkSize);
bool SendFolders(PipeFD* fds,int numWorkers,List* folders,int chunkSize);
bool GetFolderItems(char* input_dir, List* folderItemsList);
bool GetFolderPaths(List* folders, int fd, int buffSize);
bool IsFolder(struct dirent* dir);

void DestroyWorkerFolderArrList(List* arr,int size);
void GetWorkerFolderArrList(int numWorkers, List subFolderNames, List **workerFoldersArrList);

#endif
