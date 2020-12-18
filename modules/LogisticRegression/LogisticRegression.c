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

double LinearFunction(double* weights, double bWeight, double* leftVector, double* rightVector, int dimension){
    double result = bWeight;
    for (int i = 0; i < dimension/2; i++) {
        result += weights[i] * leftVector[i];
    }

    for (int i = 0; i < dimension/2; i++) {
        result += weights[dimension/2 + i] * rightVector[i];
    }

    return result;
}

double SigmoidFunction(double x){
    double val = 1.0 / (1.0 + exp(-x));
    return val;
}

double PartialDerivative(LogisticRegression* model, int derivativeIndex){
    double result = 0;

    for (int i = 0; i < model->height; i++) {
        double* leftVector = model->xVals[model->xIndexes[i][0]];
        double* rightVector = model->xVals[model->xIndexes[i][1]];
        double xij;
        if(derivativeIndex < model->width / 2){
            xij = leftVector[derivativeIndex];
        }else{
            xij = rightVector[derivativeIndex - model->width/2];
        }

        if(xij > 0){
            result += (SigmoidFunction(LinearFunction(model->weights, model->bWeight, leftVector, rightVector, model->width)) - model->yVals[i]) * xij;
        }

    }

    return result;
}

void GradientVector(LogisticRegression* model, double* vector){
    for (int i = 0; i < model->width; i++) {
        vector[i] = PartialDerivative(model, i);
        //printf("Partial derivative (%d) : %.14f\n", i, vector[i]);
    }
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

    double* newW = malloc(model->width * sizeof(double));
    double* gradientVector = malloc(model->width * sizeof(double));
    while(shouldTrain) {
        GradientVector(model, gradientVector);

        for (int i = 0; i < model->width; ++i) {
            newW[i] = model->weights[i] - learningRate * gradientVector[i];
        }
        
        double* temp = model->weights;
        model->weights = newW;
        newW = temp;

        //Compute Euclidean Distance of the 2 vectors and sets shouldTrain 
        //to false if it is smaller than terminationValue
        double dist = EuclideanDistance(newW, model->weights, model->width);
        //printf("Distance is %.15f\n", dist);
        printf("DIFF IS %.15f\n", dist - terminationValue);

        if(dist < terminationValue){
            shouldTrain = false;
        }

    }

    free(gradientVector);
    free(newW);
}

double LogisticRegression_Predict(LogisticRegression* model, double* leftVector, double* rightVector){
    return SigmoidFunction(LinearFunction(model->weights, model->bWeight, leftVector, rightVector, model->width));
}