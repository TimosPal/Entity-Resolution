#include "Hash.h"
#include "acutest.h"

typedef struct KeyValuePair{
    void* key;
    void* value;
}KeyValuePair;

bool Compare_Int(void* a,void* b){
    return *((int*)a) == *((int*)b);
}

unsigned int SimpleHash(const void* val, unsigned int size){
    return 0;
}

void Hash_Test_Init() {
    Hash hash;
    Hash_Init(&hash,100,SimpleHash,Compare_Int);

    TEST_ASSERT(hash.bucketSize == 100);
    TEST_ASSERT(hash.hashFunction == SimpleHash);
    TEST_ASSERT(hash.cmpFunction == Compare_Int);

    Hash_Destroy(hash);
}

void Hash_Test_GetValue() {
    Hash hash;
    Hash_Init(&hash,100,SimpleHash,Compare_Int);
    int* key = malloc(sizeof(int));
    *key = 12;
    int* val = malloc(sizeof(int));
    *val = 153;
    Hash_Add(&hash,key,sizeof(int),val);

    void* retVal = Hash_GetValue(hash,key,sizeof(int));
    TEST_ASSERT(*(int*)retVal == 153);

    free(key);
    free(val);
    Hash_Destroy(hash);
}

void Hash_Test_Add() {
    Hash hash;
    Hash_Init(&hash,100,SimpleHash,Compare_Int);
    int* key = malloc(sizeof(int));
    *key = 12;
    int* val = malloc(sizeof(int));
    *val = 153;
    TEST_ASSERT(Hash_Add(&hash,key,sizeof(int),val) == true);

    List list = (List)(hash.buckets[0]);
    KeyValuePair kvp = *(KeyValuePair*)list.head->value;
    TEST_ASSERT(*(int*)kvp.value == 153);

    int* key2 = malloc(sizeof(int));
    *key2 = 123;
    Hash_Add(&hash,key2,sizeof(int),val);

    kvp = *(KeyValuePair*)list.head->next->value;
    TEST_ASSERT(*(int*)kvp.value == 153);

    TEST_ASSERT(Hash_Add(&hash,key2,sizeof(int),val) == false);

    free(key);
    free(key2);
    free(val);

    Hash_Destroy(hash);
}

TEST_LIST = {
        { "Hash_Test_Init",     Hash_Test_Init },
        { "Hash_Test_Add",      Hash_Test_Add},
        { "Hash_Test_GetValue", Hash_Test_GetValue},
        { NULL, NULL }
};