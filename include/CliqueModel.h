#ifndef CLIQUE_MODEL_H
#define CLIQUE_MODEL_H

#include "CliqueGroup.h"
#include "TF-IDF.h"
#include "LogisticRegression.h"

#define DICTIONARY_TRIM_VALUE 1000

typedef struct CliqueModel{
    Hash dictionary;
    LogisticRegression logisticRegression;
}CliqueModel;

void CliqueModel_Init(CliqueModel* cliqueModel, Clique clique, Hash itemProcessedWords);
void CliqueModel_Destroy(CliqueModel* cliqueModel);

#endif