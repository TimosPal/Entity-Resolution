#include "TF-IDF.h"

#include <assert.h>
#include <math.h>

#include "Util.h"
#include "Hashes.h"
#include "StringUtil.h"
#include "Tuple.h"

void InsertIcpWordsToDict(ItemCliquePair icp, Hash* dictionary, Hash processedWords){
    /* For every item in the clique, inserts every one of its unique words to the dictionary,
     * or increments the counter if it already exists
     * This is a helper function for TF_IDF_Calculate */

    //Assisting Hash to not increment counters 2 times for the same words, prevents duplicating
    Hash currCliqueWordsInserted;
    Hash_Init(&currCliqueWordsInserted,DEFAULT_HASH_SIZE,RSHash,StringCmp);

    //Get the list of words for the specific icp
    List* words = Hash_GetValue(processedWords, &icp.id, sizeof(icp.id));

    //For each word in the list
    Node* currWordNode = words->head;
    while(currWordNode != NULL){
        char* currWord = currWordNode->value;
        int keySize = (int)strlen(currWord) + 1;

        //If it has not yet been counted for this word list(meaning it is included 2 or more times in the list)
        if(!Hash_GetValue(currCliqueWordsInserted,currWord,keySize)){
            double* count = Hash_GetValue(*dictionary,currWord,keySize);
            //If it is has not been found in any other icps, malloc it and increment
            if(!count){
                count = malloc(sizeof(double));
                *count = 1;
                Hash_Add(dictionary, currWord, keySize, count);
            }else{ //just increment
                (*count)++;
            }

            //Add it to assisting hash so we will not increment it again(uniquely increment word counts)
            Hash_Add(&currCliqueWordsInserted,currWord,keySize,"-");
        }

        currWordNode = currWordNode->next;
    }

    Hash_Destroy(currCliqueWordsInserted);
}

Hash CreateDictionary(List correlated, Hash processedWords){
    /* Creates a dictionary of all the unique words for every item
     * that is correlated to this clique. */

    Hash dictionary;
    Hash_Init(&dictionary,DEFAULT_HASH_SIZE,RSHash,StringCmp);

    //Insert unique words to dict for each icp correlated to the clique
    Node* currIcpNode = correlated.head;
    while(currIcpNode != NULL){
        ItemCliquePair icp = *(ItemCliquePair*)currIcpNode->value;
        InsertIcpWordsToDict(icp, &dictionary, processedWords);

        currIcpNode = currIcpNode->next;
    }

    return dictionary;
}

int IDF_Index_Cmp(const void* value1, const void* value2){
    KeyValuePair* kvp1 = *(KeyValuePair**)value1;
    KeyValuePair* kvp2 = *(KeyValuePair**)value2;

    double idf1 = *(double*)(kvp1->value);
    double idf2 = *(double*)(kvp2->value);

    if (idf1 > idf2){
        return -1;
    }else if(idf1 < idf2){
        return 1;
    }else{
        return 0;
    }
}

Hash IDF_Calculate(List correlated, Hash proccesedWords, int dimensionLimit){
    // Calculated IDF value for current dictionary.

    //Get the dictionary based on those icps
    Hash dictionary = CreateDictionary(correlated, proccesedWords);

    int numberOfItems = correlated.size;

    Node* currWordCountNode  = dictionary.keyValuePairs.head;
    while(currWordCountNode != NULL){
        double* val = ((KeyValuePair*)currWordCountNode->value)->value;
        *val = log(numberOfItems / *val);

        currWordCountNode = currWordCountNode->next;
    }

    // Remove small frequencies for smaller dimensionality.

    // Convert List to Array
    KeyValuePair** kvpArray = (KeyValuePair**)List_ToArray(dictionary.keyValuePairs);

    //Sorting the array based on IDF values(descending)
    qsort(kvpArray, dictionary.keyValuePairs.size, sizeof(KeyValuePair*), IDF_Index_Cmp);
    for (int j = 0; j < 100; ++j) {
        printf("%s \n",kvpArray[j]->key);
    }

    //Now Trim
    Hash trimmedDictionary;
    Hash_Init(&trimmedDictionary, DEFAULT_HASH_SIZE, RSHash, StringCmp);

    if (dimensionLimit >= dictionary.keyValuePairs.size || dimensionLimit == -1){
        dimensionLimit = dictionary.keyValuePairs.size;
    }

    for (int i = 0; i < dimensionLimit; i++){
        Tuple* double_index_tuple = malloc(sizeof(Tuple));

        //Malloc and set values for tuple ( <double, int> )
        Tuple_Init(double_index_tuple, kvpArray[i]->value, sizeof(double), &i, sizeof(i));

        // add tuple to new dictionary
        int keySize = strlen(kvpArray[i]->key) + 1;
        Hash_Add(&trimmedDictionary, kvpArray[i]->key, keySize, double_index_tuple);
    }

    free(kvpArray);
    Hash_FreeValues(dictionary, free);
    Hash_Destroy(dictionary);

    return trimmedDictionary;
}

double* TF_IDF_Calculate(Hash dictionary, List processedWords){
    /* Calculates a tfidf vector for said Word list based on
     * the given dictionary */
    int vectorSize = dictionary.keyValuePairs.size;

    double* vector = calloc(vectorSize, sizeof(double));

    //Calculating BoW
    //Dictionary Hash has the indexes as values that are used in the vector
    Node* currWordNode = processedWords.head;
    while(currWordNode != NULL){
        int keySize = strlen(currWordNode->value)+1;
        Tuple* double_index_tuple = (Tuple*) Hash_GetValue(dictionary, currWordNode->value, keySize);
        //if word is in dictionary
        if (double_index_tuple){
            int index = *(int*)double_index_tuple->value2;
            vector[index]++;
        }

        currWordNode = currWordNode->next;
    }

    //Calculate TF-IDF for every number in vector
    Node* currKVPNode = dictionary.keyValuePairs.head;
    while(currKVPNode != NULL){
        KeyValuePair* kvp = currKVPNode->value;
        Tuple* double_index_tuple = (Tuple*)kvp->value;

        int index = *(int*)double_index_tuple->value2;

        vector[index] /= processedWords.size;
        vector[index] *= *(double*)double_index_tuple->value1;

        currKVPNode = currKVPNode->next;
    }

    return vector;
}

