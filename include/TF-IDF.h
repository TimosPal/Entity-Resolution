#ifndef TF_IDF_H
#define TF_IDF_H

#include "Hash.h"
#include "CliqueGroup.h"

Hash IDF_Calculate(List items, Hash proccesedWords, int dimensionLimit);
Hash CreateVector(Hash dictionary, List processedWords);
double* TF_IDF_ToArray(Hash hash, Hash dictionary);

Hash* CreateVectors(List xVals, Hash dictionary, Hash itemProcessedWords);
double* CreateY(List pairs);


#endif