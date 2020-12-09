#ifndef LOGISTICREGRESSION_H
#define LOGISTICREGRESSION_H

typedef struct LogisticRegression {
    double* weights;
    double bWeight;

    double** xVals;
    double* yVals;

    int width;
    int height;
} LogisticRegression;

#endif