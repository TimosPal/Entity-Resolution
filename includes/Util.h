#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

#define ERROR 1
#define IF_ERROR(x) if(x) { exit(ERROR); }
#define IF_ERROR_MSG(x,msg) if(x) { perror(msg); exit(ERROR); }

#endif