#ifndef LOGISTICREGRESSION_H
#define LOGISTICREGRESSION_H

typedef struct LogisticRegression {
    double* weights;
    double bWeight;

    double** xVals;
    double* yVals;
    unsigned int** xIndexes; //for pairs


    unsigned int width;
    unsigned int height;
    unsigned int itemCount;
} LogisticRegression;

void LogisticRegression_Init(LogisticRegression* model, double bWeight, double** xVals, unsigned int** xIndexes, double* yVals, unsigned int width, unsigned int height, unsigned int itemCount);
void LogisticRegression_Destroy(LogisticRegression model);
void LogisticRegression_Train(LogisticRegression* model, double learningRate, double terminationValue);
double LogisticRegression_Predict(LogisticRegression* model, double* vector);

#endif