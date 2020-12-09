#include "LogisticRegression.h"

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

double LinearFunction(double* weights,double bWeight,double* xVals,int dimension){
    double result = bWeight;
    for (int i = 0; i < dimension; ++i) {
        result += weights[i] * xVals[i];
    }

    return result;
}

double SigmoidFunction(double x){
    return 1.0 / (1.0 + exp(-x));
}

double PartialDerivative(double* weights, double bWeight, double** xVals, double* yVals, int height, int width, int derivativeIndex){
    double result = 0;

    for (int i = 0; i < height; ++i) {
        result += (SigmoidFunction(LinearFunction(weights,bWeight,xVals[i],width)) - yVals[i]) * xVals[i][derivativeIndex];
    }

    return result;
}

double* GradientVector(double* weights, double bWeight, double** xVals, double* yVals, int height, int width){
    double* vector = malloc(width * sizeof(double));

    for (int i = 0; i < width; ++i) {
        vector[i] = PartialDerivative(weights, bWeight, xVals, yVals, height, width, i);
    }

    return vector;
}

void LogisticRegression_Init(LogisticRegression* model,double bWeight,double** xVals,double* yVals,int width, int height){
    model->bWeight = bWeight;

    model->xVals = xVals;
    model->yVals = yVals;

    model->height = height;
    model->width = width;

    model->weights = calloc(width, sizeof(double));
}

void LogisticRegression_Train(LogisticRegression* model, double learningRate, double terminationValue){
    bool shouldTrain = true;

    while(shouldTrain) {
        double *gradientVector = GradientVector(
                model->weights,
                model->bWeight,
                model->xVals,
                model->yVals,
                model->height,
                model->width);

        for (int i = 0; i < model->width; ++i) {
            double prevWi = model->weights[i];
            model->weights[i] -= learningRate * gradientVector[i];

            if(model->weights[i] - prevWi < terminationValue)
                shouldTrain = false;
        }

        free(gradientVector);
    }
}

double LogisticRegression_Predict(LogisticRegression* model, double* xVals){
    return SigmoidFunction(LinearFunction(model->weights,model->bWeight,xVals,model->width));
}