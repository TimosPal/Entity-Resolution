#ifndef HASHES_H_INCLUDED
#define HASHES_H_INCLUDED

/* SOURCE : https://www.partow.net/programming/hashfunctions/ */

unsigned int RSHash(const char*, unsigned int);
unsigned int SDBMHash(const char*, unsigned int);
unsigned int APHash(const char*, unsigned int);

#endif
