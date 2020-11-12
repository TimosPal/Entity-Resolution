#include "acutest.h"
#include "JSONParser.h"


void JSONParser_Test_GetStringBetweenQuotes(){
    char* filePath = "testFile.txt";
    FILE* fp = fopen(filePath, "w+");
    char* testString = "qwertyuiopasdfghjklzxcvbnm,./;'[]}{1234567890''\"";
    fwrite(testString, 1, strlen(testString) + 1, fp);
    rewind(fp);

    char* newStr = GetStringBetweenQuotes(fp);

    char* outputExpected = "qwertyuiopasdfghjklzxcvbnm,./;'[]}{1234567890''";

    TEST_ASSERT(strcmp(newStr, outputExpected) == 0);

    free(newStr);
    remove(filePath);
}


void JSONParser_Test_GetJsonPairs(){
    char* filePath = "testFile.json";
    FILE* fp = fopen(filePath, "w+");

    char* testString = "{\"<page title>\": \"Olympus OM-D E-M10 Black Digital Camera (16.1 MP, SD/SDHC/SDXC Card Slot) Price Comparison at Buy.net\",\"camera body only\": \"Body Only\",}";
    char* expectedLeftVal1 = "<page title>";
    char* expectedRightVal1 = "Olympus OM-D E-M10 Black Digital Camera (16.1 MP, SD/SDHC/SDXC Card Slot) Price Comparison at Buy.net";
    char* expectedLeftVal2 = "camera body only";
    char* expectedRightVal2 = "Body Only";

    fwrite(testString, 1, strlen(testString) + 1, fp);
    rewind(fp);

    List pairs = GetJsonPairs(filePath);

    ValuePair* firstPair = pairs.head->value;

    TEST_ASSERT(firstPair->leftVal != NULL);
    char* leftVal1 = (char*)(firstPair->leftVal); 
    TEST_ASSERT(strcmp(leftVal1, expectedLeftVal1) == 0);

    TEST_ASSERT(firstPair->rightVals.head->value != NULL);
    char* rightVal1 = (char*)(firstPair->rightVals.head->value); 
    TEST_ASSERT(strcmp(rightVal1, expectedRightVal1) == 0);

    ValuePair* secondPair = pairs.tail->value;

    TEST_ASSERT(secondPair->leftVal != NULL);
    char* leftVal2 = (char*)(secondPair->leftVal); 
    TEST_ASSERT(strcmp(leftVal2, expectedLeftVal2) == 0);

    TEST_ASSERT(secondPair->rightVals.head->value != NULL);
    char* rightVal2 = (char*)(secondPair->rightVals.head->value); 
    TEST_ASSERT(strcmp(rightVal2, expectedRightVal2) == 0);    

    List_FreeValues(pairs, ValuePair_Free);
    List_Destroy(&pairs);

    remove(filePath);

}



TEST_LIST = {
    { "JSONParser_Test_GetStringBetweenQuotes",   JSONParser_Test_GetStringBetweenQuotes },
    { "JSONParser_Test_GetJsonPairs",   JSONParser_Test_GetJsonPairs },
    { NULL, NULL }
};