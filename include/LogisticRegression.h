#ifndef LOGISTICREGRESSION_H
#define LOGISTICREGRESSION_H

#include "Hash.h"

typedef struct LogisticRegression {
    double* weights;
    double bWeight;

    Hash* xVals;


    unsigned int width;
    unsigned int itemCount;
} LogisticRegression;

void LogisticRegression_Init(LogisticRegression* model, double bWeight, Hash* xVals, unsigned int width, unsigned int itemCount);
void LogisticRegression_Destroy(LogisticRegression model);
double LogisticRegression_Predict(LogisticRegression* model, Hash* leftVector, Hash* rightVector);
double* LogisticRegression_Train(LogisticRegression *model,unsigned int** xIndexes, double* yVals, unsigned int height, double learningRate, int epochs, int batchSize);

#endif