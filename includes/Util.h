#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "ArgUtil.h"
#include "FolderUtil.h"
#include "StringUtil.h"

#define BUFFER_SIZE 256
#define EXIT_ERROR_CODE 1

#define IF_ERROR_MSG(x,msg) { if(x) {fprintf(stderr,"ERROR : %s.\n",msg); exit(EXIT_ERROR_CODE); } }

#endif