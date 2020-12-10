#include "CliqueModel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Tuple.h"
#include "LogisticRegression.h"


void CliqueModel_CreateXY(Clique clique, Hash dictionary, Hash itemProcessedWords, double*** x, double** y, int* width, int* height){
    /* Gets all tf_idf vectors for every icp correlated to the clique */

    List correlatedIcps = Clique_GetCorrelatedIcps(clique);

    *width = dictionary.keyValuePairs.size;
    *height = correlatedIcps.size;

    double **vectors = malloc(*width * sizeof(double*));
    double* results = malloc(*height * sizeof(double));

    int index = 0;
    Node* currIcpNode = correlatedIcps.head;
    while (currIcpNode != NULL){
        ItemCliquePair* icp = (ItemCliquePair*)currIcpNode->value;
        List* processedWords = Hash_GetValue(itemProcessedWords, &icp->id, sizeof(icp->id));
        assert(processedWords != NULL);

        //X
        double* vector = TF_IDF_Calculate(dictionary, *processedWords); // this mallocs
        vectors[index] = vector;

        //Y
        results[index] = (icp->clique->id == clique.id) ?  1 :  0;

        index++;
        currIcpNode = currIcpNode->next;
    }

    *x = vectors;
    *y = results;

    //Cleanup
    List_Destroy(&correlatedIcps);

}

void CliqueModel_Init(CliqueModel* cliqueModel, Clique clique, Hash itemProcessedWords){
    cliqueModel->dictionary = IDF_Calculate(clique, itemProcessedWords, DICTIONARY_TRIM_VALUE);
    printf("Dictionary Size is %d\n", cliqueModel->dictionary.keyValuePairs.size);

    //calculate tf_idf ( the X of the model )
    int width, height;
    double** x;
    double* y;

    CliqueModel_CreateXY(clique, cliqueModel->dictionary, itemProcessedWords, &x, &y, &width, &height);

    //now logistic regression
    LogisticRegression logisticRegression;
    LogisticRegression_Init(&logisticRegression, 0, x, y, width, height);
    //set the logisticRegression in the cliqueModel
    cliqueModel->logisticRegression = logisticRegression;

    //train

}

void CliqueModel_Destroy(CliqueModel* cliqueModel){
    Hash_FreeValues(cliqueModel->dictionary, Tuple_Free);
    Hash_Destroy(cliqueModel->dictionary);

    LogisticRegression_Destroy(cliqueModel->logisticRegression);
}