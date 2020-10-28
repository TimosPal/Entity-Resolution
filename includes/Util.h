#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define ERROR 1

#define IF_ERROR(x) if(x) { exit(ERROR); }
#define IF_ERROR_MSG(x,msg) if(x) { (errno != 0) ? perror(msg) : printf("%s",msg); exit(ERROR); }

#endif