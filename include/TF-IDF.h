#ifndef TF_IDF_H
#define TF_IDF_H

#include "Hash.h"
#include "CliqueGroup.h"

Hash CreateIDF(Clique clique, Hash proccesedWords, int dimensionLimit);

#endif