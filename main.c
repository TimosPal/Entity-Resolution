#include <stdio.h>
#include <dirent.h>
#include "Util.h"
#include "ArgUtil.h"

int main(int argc, char* argv[]){

    // Get the flags from argv.
    // -f should contain the path of a folder of folders.
    char *folderName;
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-f", &folderName), "Invalid folder path\n")

    // Open the folders.
    DIR *dir_ptr;
    struct dirent *direntp;

    return 0;
}