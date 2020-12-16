#ifndef TF_IDF_H
#define TF_IDF_H

#include "Hash.h"
#include "CliqueGroup.h"

Hash IDF_Calculate(CliqueGroup cliqueGroup, Hash proccesedWords, int dimensionLimit);
Hash TF_IDF_Calculate(Hash dictionary, List processedWords);
double* TF_IDF_ToArray(Hash hash, Hash dictionary);

Hash* CreateX(List xVals, Hash dictionary, Hash itemProcessedWords);
double* CreateY(List pairs);


#endif