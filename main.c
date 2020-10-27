#include <stdio.h>
#include <dirent.h>
#include "./includes/Util.h"
#include "./includes/ArgUtil.h"

int main(int argc, char* argv[]){

    //GET FLAGS
    //-f FLAG
    char *folderName;
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-f", &folderName), "Name of folder is not valid\n")

    //PARSE FILES
    DIR *dir_ptr;
    struct dirent *direntp;

    


    return 0;
}