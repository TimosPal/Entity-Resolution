#include "TF-IDF.h"

#include <math.h>

#include "Item.h"
#include "Util.h"
#include "Hashes.h"
#include "StringUtil.h"

void InsertCliqueWordsToDict(Clique clique, Hash* dictionary, Hash processedWords){
    /* For every item in the clique, inserts every one of its unique words to the dictionary,
     * or increments the counter if it already exists
     * This is a helper function for TF_IDF */

    Node* currItemNode = clique.similar.head;
    while(currItemNode != NULL){
        Hash currCliqueWordsInserted;
        Hash_Init(&currCliqueWordsInserted,DEFAULT_HASH_SIZE,RSHash,StringCmp);

        Item* currItem = ((ItemCliquePair*)currItemNode->value)->item;

        List* words = Hash_GetValue(processedWords,currItem->id,strlen(currItem->id) + 1);
        Node* currWordNode = words->head;
        while(currWordNode != NULL){
            char* currWord = currWordNode->value;
            int keySize = (int)strlen(currWord) + 1;

            if(!Hash_GetValue(currCliqueWordsInserted,currWord,keySize)){
                double* count = Hash_GetValue(*dictionary,currWord,keySize);
                if(!count){
                    count = malloc(sizeof(double));
                    *count = 1;
                    Hash_Add(dictionary,currWord,keySize,count);
                }else{
                    (*count)++;
                }

                Hash_Add(&currCliqueWordsInserted,currWord,keySize,"-");
            }

            currWordNode = currWordNode->next;
        }

        Hash_Destroy(currCliqueWordsInserted);

        currItemNode = currItemNode->next;
    }
}

Hash CreateDictionary(Clique clique, Hash processedWords){
    /* Creates a dictionary of all the unique words for every item
     * that is correlated to this clique. */

    Hash dictionary;
    Hash_Init(&dictionary,DEFAULT_HASH_SIZE,RSHash,StringCmp);

    InsertCliqueWordsToDict(clique,&dictionary,processedWords);
    List nonSimilarCliques = clique.nonSimilar;

    Node* currNonSimilarCliqueNode = nonSimilarCliques.head;
    while(currNonSimilarCliqueNode != NULL){
        Clique* currNonSimilarClique = ((ItemCliquePair*)currNonSimilarCliqueNode->value)->clique;
        InsertCliqueWordsToDict(*currNonSimilarClique,&dictionary,processedWords);

        currNonSimilarCliqueNode = currNonSimilarCliqueNode->next;
    }

    return dictionary;
}

int GetNumberOfItems(Clique* clique){
    /* Calculates the number of items in the
     * similar and non similar cliques.
     * Helper function for TF. */

    int sum = clique->similar.size;

    Node* currNonSimilarCliqueNode = clique->nonSimilar.head;
    while(currNonSimilarCliqueNode != NULL){
        Clique* currNonSimilarClique = ((ItemCliquePair*)currNonSimilarCliqueNode->value)->clique;

        sum += currNonSimilarClique->similar.size;

        currNonSimilarCliqueNode = currNonSimilarCliqueNode->next;
    }

    return sum;
}

Hash CreateIDF(Clique clique, Hash proccesedWords, int dimensionLimit){
    // Calculated IDF value for current dictionary.
    Hash dictionary = CreateDictionary(clique, proccesedWords);

    int numberOfItems = GetNumberOfItems(&clique);

    Node* currWordCountNode  = dictionary.keyValuePairs.head;
    while(currWordCountNode != NULL){
        double* val = ((KeyValuePair*)currWordCountNode->value)->value;
        *val = log(numberOfItems / *val);

        printf("noi %d count %f\n",numberOfItems,*val);

        currWordCountNode = currWordCountNode->next;
    }

    // Remove small frequencies for smaller dimensionality.

    // If no trimming is possible
    if (dimensionLimit >= dictionary.keyValuePairs.size || dimensionLimit == -1){
        return dictionary;
    }

    // Convert List to Array
    KeyValuePair** kvpArray = (KeyValuePair**)List_ToArray(dictionary.keyValuePairs);

    //Sorting the array based on IDF values


    //Now Trim
    Hash trimmedDictionary;
    Hash_Init(&trimmedDictionary,DEFAULT_HASH_SIZE,RSHash,StringCmp);

    for (int i = 0; i < dimensionLimit; i++){
        Tuple* double_index = malloc(sizeof(Tuple));
        int* index = malloc(sizeof(int));

        Hash_Add(&trimmedDictionary, kvpArray[i]->key, (int)strlen(kvpArray[i]->key) + 1, kvpArray[i]->value);
    }

    Hash_FreeValues(dictionary, free);
    Hash_Destroy(dictionary);

    return trimmedDictionary;
}