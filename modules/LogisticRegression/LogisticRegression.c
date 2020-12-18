#include "LogisticRegression.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

double EuclideanDistance(double* x, double* y, unsigned int size){
    double sum = 0;
    for (int i = 0; i < size; i++){
        sum += (x[i] - y[i]) * (x[i] - y[i]);
    }

    return sqrt(sum);
}

double LinearFunction(double* weights, double bWeight, double* xVals, int dimension){
    double result = bWeight;
    for (int i = 0; i < dimension; ++i) {
        result += weights[i] * xVals[i];
    }

    return result;
}

double SigmoidFunction(double x){
    double val = 1.0 / (1.0 + exp(-x));
    return val;
}

double PartialDerivative(LogisticRegression* model, int derivativeIndex){
    double result = 0;

    for (int i = 0; i < model->height; ++i) {
        double* vector = malloc(model->width * sizeof(double));
        memcpy(vector, model->xVals[model->xIndexes[i][0]], model->width/2 * sizeof(double));
        memcpy(vector + model->width/2, model->xVals[model->xIndexes[i][1]], model->width/2 * sizeof(double));

        if(vector[derivativeIndex] > 0){
            result += (SigmoidFunction(LinearFunction(model->weights, model->bWeight, vector, model->width)) - model->yVals[i]) * vector[derivativeIndex];
        }

        free(vector);
    }

    return result;
}

double* GradientVector(LogisticRegression* model){
    double* vector = malloc(model->width * sizeof(double));

    for (int i = 0; i < model->width; i++) {
        vector[i] = PartialDerivative(model, i);
        //printf("Partial derivative (%d) : %.14f\n", i, vector[i]);
    }

    return vector;
}

void LogisticRegression_Init(LogisticRegression* model, double bWeight, double** xVals, unsigned int** xIndexes, double* yVals, unsigned int width, unsigned int height, unsigned int itemCount){
    model->bWeight = bWeight;

    model->xVals = xVals;
    model->xIndexes = xIndexes;
    model->yVals = yVals;

    model->height = height;
    model->itemCount = itemCount;
    model->width = width;

    //setting the starting weights
    model->weights = calloc(width, sizeof(double));
    for (int i = 0; i < width; i++){
        model->weights[i] = 1.0;
    }
}

void LogisticRegression_Destroy(LogisticRegression model){
    free(model.weights);
    free(model.yVals);
    
    for(int i = 0; i < model.itemCount; i++){
        free(model.xVals[i]);
    }
    for(int i = 0; i < model.height; i++){
        free(model.xIndexes[i]);
    }
    free(model.xVals);
    free(model.xIndexes);
}

void LogisticRegression_Train(LogisticRegression* model, double learningRate, double terminationValue){
    bool shouldTrain = true;

    while(shouldTrain) {
        double* gradientVector = GradientVector(model);

        double* newW = malloc(model->width * sizeof(double));
        for (int i = 0; i < model->width; ++i) {
            newW[i] = model->weights[i] - learningRate * gradientVector[i];
        }
        
        //Compute Euclidean Distance of the 2 vectors and sets shouldTrain 
        //to false if it is smaller than terminationValue
        double dist = EuclideanDistance(newW, model->weights, model->width);
        //printf("Distance is %.15f\n", dist);
        printf("DIFF IS %.15f\n", dist - terminationValue);

        if(dist < terminationValue){
            shouldTrain = false;
        }

        free(model->weights);
        model->weights = newW;

        free(gradientVector);
    }
}

double LogisticRegression_Predict(LogisticRegression* model, double* vector){
    return SigmoidFunction(LinearFunction(model->weights, model->bWeight, vector, model->width));
}