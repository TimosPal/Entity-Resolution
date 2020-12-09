#ifndef TF_IDF_H
#define TF_IDF_H

#include "Hash.h"
#include "CliqueGroup.h"

Hash IDF_Calculate(Clique clique, Hash proccesedWords, int dimensionLimit);

double* TF_IDF_Calculate(Hash dictionary, List processedWords);

#endif