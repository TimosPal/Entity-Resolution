#include "CliqueModel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Tuple.h"
#include "LogisticRegression.h"


/* Gets all tf_idf vectors for every icp correlated to the clique */
void CliqueModel_CreateXY(List correlated, Clique clique, Hash dictionary, Hash itemProcessedWords, double*** x, double** y, unsigned int* width, unsigned int* height){

    //Set width and height
    *width = (unsigned int)dictionary.keyValuePairs.size;
    *height = (unsigned int)correlated.size;

    //Malloc arrays for X and Y data
    double **vectors = malloc(*height * sizeof(double*));
    double* results = malloc(*height * sizeof(double));

    int index = 0;
    Node* currIcpNode = correlated.head;
    while (currIcpNode != NULL){
        ItemCliquePair* icp = (ItemCliquePair*)currIcpNode->value;
        List* processedWords = Hash_GetValue(itemProcessedWords, &icp->id, sizeof(icp->id));
        assert(processedWords != NULL);

        //X
        double* vector = TF_IDF_Calculate(dictionary, *processedWords); // this mallocs
        vectors[index] = vector;

        //Y
        results[index] = (icp->clique->id == clique.id) ?  1.0 :  0.0;

        index++;
        currIcpNode = currIcpNode->next;
    }

    *x = vectors;
    *y = results;
}

void CliqueModel_Init(CliqueModel* cliqueModel, List correlated, Clique clique, Hash itemProcessedWords){
    //Calculate IDF and insert it into the dictionary
    cliqueModel->dictionary = IDF_Calculate(correlated, itemProcessedWords, 100);

    //Calculate tf_idf ( the X of the model )
    unsigned int width, height;
    double** x;
    double* y;
    CliqueModel_CreateXY(correlated, clique, cliqueModel->dictionary, itemProcessedWords, &x, &y, &width, &height);

    //Now logistic regression
    LogisticRegression logisticRegression;
    LogisticRegression_Init(&logisticRegression, 0, x, y, width, height);
    //Set the logisticRegression in the cliqueModel
    cliqueModel->logisticRegression = logisticRegression;

    //Train
    printf("Dictionary Size is %d, XY training size is %u\n", cliqueModel->dictionary.keyValuePairs.size, cliqueModel->logisticRegression.height);
    LogisticRegression_Train(&cliqueModel->logisticRegression, 10, 1);

    for (int i = 0; i < height; i++){
        double prediction = LogisticRegression_Predict(&cliqueModel->logisticRegression, x[i]);
        printf("Prediction: %.15f, Actual: %f\n", prediction, y[i]);
    }
}

void CliqueModel_Destroy(CliqueModel* cliqueModel){
    Hash_FreeValues(cliqueModel->dictionary, Tuple_Free);
    Hash_Destroy(cliqueModel->dictionary);

    LogisticRegression_Destroy(cliqueModel->logisticRegression);
}