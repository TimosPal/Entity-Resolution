#ifndef HASHES_H
#define HASHES_H

/* SOURCE : https://www.partow.net/programming/hashfunctions/ */

unsigned int RSHash(const void*, unsigned int);
unsigned int SDBMHash(const void*, unsigned int);
unsigned int APHash(const void*, unsigned int);

#endif
