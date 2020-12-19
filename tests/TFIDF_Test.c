#include "acutest.h"

#include "TF-IDF.h"
#include "Util.h"
#include "Hashes.h"
#include <math.h>

bool UIntCmp(void* value1, void* value2){
    if(*(unsigned int*)value1 == *(unsigned int*)value2){
        return true;
    }else{
        return false;
    }
}

void TFID_Test_IDF_Calculate(){
    List items;
    List_Init(&items);

    ItemCliquePair icp1 = { .id = 1};
    ItemCliquePair icp2 = { .id = 2};
    ItemCliquePair icp3 = { .id = 3};

    List_Append(&items, &icp1);
    List_Append(&items, &icp2);
    List_Append(&items, &icp3);

    char w1[] = "word1";
    char w2[] = "word2";
    char w3[] = "word3";

    List text1; //
    List_Init(&text1);
    List_Append(&text1,w1);

    List text2;
    List_Init(&text2);
    List_Append(&text2,w1);
    List_Append(&text2,w2);

    List text3;
    List_Init(&text3);
    List_Append(&text3,w1);
    List_Append(&text3,w2);
    List_Append(&text3,w3);

    Hash words;
    Hash_Init(&words, 10, RSHash, UIntCmp, false);
    Hash_Add(&words, &icp1.id, sizeof(unsigned int), &text1);
    Hash_Add(&words, &icp2.id, sizeof(unsigned int), &text2);
    Hash_Add(&words, &icp3.id, sizeof(unsigned int), &text3);

    Hash idf = IDF_Calculate(items, words, 100);

    double* idfVal = Hash_GetValue(idf,w1 , strlen(w1) + 1);
    TEST_ASSERT(*idfVal == log(3.0/3.0));

    idfVal = Hash_GetValue(idf,w2 , strlen(w2) + 1);
    TEST_ASSERT(*idfVal == log(3.0/2.0));

    idfVal = Hash_GetValue(idf,w3 , strlen(w3) + 1);
    TEST_ASSERT(*idfVal == log(3.0/1.0));

    List_Destroy(&text1);
    List_Destroy(&text2);
    List_Destroy(&text3);
    List_Destroy(&items);

    Hash_Destroy(words);

    Hash_FreeValues(idf, free);
    Hash_Destroy(idf);
}

void TFIDF_Test_TFIDF_Calculate(){
    List items;
    List_Init(&items);

    ItemCliquePair icp1 = { .id = 1};
    ItemCliquePair icp2 = { .id = 2};
    ItemCliquePair icp3 = { .id = 3};

    List_Append(&items, &icp1);
    List_Append(&items, &icp2);
    List_Append(&items, &icp3);

    char w1[] = "word1";
    char w2[] = "word2";
    char w3[] = "word3";

    List text1; //
    List_Init(&text1);
    List_Append(&text1,w1);

    List text2;
    List_Init(&text2);
    List_Append(&text2,w1);
    List_Append(&text2,w2);

    List text3;
    List_Init(&text3);
    List_Append(&text3,w1);
    List_Append(&text3,w2);
    List_Append(&text3,w3);
    List_Append(&text3,w3);
    List_Append(&text3,w3);

    Hash words;
    Hash_Init(&words, 10, RSHash, UIntCmp, false);
    Hash_Add(&words, &icp1.id, sizeof(unsigned int), &text1);
    Hash_Add(&words, &icp2.id, sizeof(unsigned int), &text2);
    Hash_Add(&words, &icp3.id, sizeof(unsigned int), &text3);

    Hash idf = IDF_Calculate(items, words, 100);

    Hash vec = CreateVector(idf, text3);

    double* tfidfVal = Hash_GetValue(vec, w3, strlen(w3) + 1);
    TEST_ASSERT(*tfidfVal == 3.0/5.0 * log(3.0/1.0));

    tfidfVal = Hash_GetValue(vec, w2, strlen(w2) + 1);
    TEST_ASSERT(*tfidfVal == 1.0/5.0 * log(3.0/2.0));

    List_Destroy(&text1);
    List_Destroy(&text2);
    List_Destroy(&text3);
    List_Destroy(&items);

    Hash_Destroy(words);

    Hash_FreeValues(idf, free);
    Hash_Destroy(idf);
    Hash_FreeValues(vec, free);
    Hash_Destroy(vec);
}

TEST_LIST = {
        { "TFIDF_Test_IDF_Calculate",  TFID_Test_IDF_Calculate},
        { "TFIDF_Test_CreateVector",  TFIDF_Test_TFIDF_Calculate},
        { NULL, NULL }
};