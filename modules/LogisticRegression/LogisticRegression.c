#include "LogisticRegression.h"
#include "Util.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "JobScheduler.h"

extern JobScheduler jobScheduler;

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

double PartialDerivative(LogisticRegression* model,unsigned int** xIndexes, double* yVals, unsigned int height, int derivativeIndex, int j){
    double result = 0;
    
    int limit = j + BATCH_SIZE;
    if (limit > height){
        limit = height;
    }

    for(int k = j; k < limit; k++){
        double* leftVector = model->xVals[xIndexes[k][0]];
        double* rightVector = model->xVals[xIndexes[k][1]];
        double xij;
        if(derivativeIndex < model->width / 2){
            xij = leftVector[derivativeIndex];
        }else{
            xij = rightVector[derivativeIndex - model->width/2];
        }

        if(xij > 0){
            result += (SigmoidFunction(LinearFunction(model->weights, model->bWeight, leftVector, rightVector, model->width)) - yVals[k]) * xij;
        }
    }

    return result;
}

void GradientVector(LogisticRegression* model,unsigned int** xIndexes, double* yVals, unsigned int height, double* vector, int j){
    for (int i = 0; i < model->width; i++) {
        vector[i] = PartialDerivative(model,xIndexes, yVals, height, i, j);
        //printf("Partial derivative (%d) : %.14f\n", i, vector[i]);
    }
}

void LogisticRegression_Init(LogisticRegression* model, double bWeight, double** xVals, unsigned int width, unsigned int itemCount){
    model->bWeight = bWeight;

    model->xVals = xVals;

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
}

double* LogisticRegression_Train(LogisticRegression *model,unsigned int** xIndexes, double* yVals, unsigned int height, double learningRate, int epochs) {
    double* newW = malloc(model->width * sizeof(double));
    double* gradientVector = malloc(model->width * sizeof(double));

    int batches = (int)height / BATCH_SIZE;
    if (height % BATCH_SIZE != 0){
        batches++;
    }

    double** subWeights = malloc(jobScheduler.numberOfThreads * sizeof(double*));
    for (int l = 0; l < jobScheduler.numberOfThreads; ++l) {
        subWeights[l] = calloc(model->width, sizeof(double));
    }

    for(int k = 0; k < epochs; k++) {
        int m = 0;
        while (m < batches) {
            int numberOfVectors = jobScheduler.numberOfThreads;
            if(numberOfVectors >= batches - m){
                numberOfVectors = batches - m;
            }

            // Calculate #threads sub-weights for each batch of train batches.
            for (int j = 0; j < numberOfVectors; j++) {
                GradientVector(model, xIndexes, yVals, height, subWeights[j], m++ * BATCH_SIZE);
            }


            for (int l = 0; l < model->width; ++l) {
                double sum = 0;
                for (int i = 0; i < numberOfVectors; ++i) {
                    sum += subWeights[i][l];
                }
                gradientVector[l] = sum / (double)numberOfVectors;
            }

            for (int i = 0; i < model->width; ++i) {
                newW[i] = model->weights[i] - learningRate * gradientVector[i];
            }

            double *temp = model->weights;
            model->weights = newW;
            newW = temp;
        }

        printf("\r%d out of %d epochs", k+1, epochs);
        fflush(stdout);
    }

    free(gradientVector);
    free(newW);

    for (int l = 0; l < jobScheduler.numberOfThreads; ++l) {
        free(subWeights[l]);
    }
    free(subWeights);

    return model->weights;
}

double LogisticRegression_Predict(LogisticRegression* model, double* leftVector, double* rightVector){
    return SigmoidFunction(LinearFunction(model->weights, model->bWeight, leftVector, rightVector, model->width));
}