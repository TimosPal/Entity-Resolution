#ifndef HASHES_H_INCLUDED
#define HASHES_H_INCLUDED

/* SOURCE : https://www.partow.net/programming/hashfunctions/ */

unsigned int RSHash(const void*, unsigned int);
unsigned int SDBMHash(const void*, unsigned int);
unsigned int APHash(const void*, unsigned int);

#endif
