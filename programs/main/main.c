#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <assert.h>
#include <math.h>

#include "Util.h"
#include "ArgUtil.h"
#include "FolderUtil.h"
#include "StringUtil.h"

#include "JSONParser.h"
#include "CSVParser.h"

#include "Hashes.h"
#include "CliqueGroup.h"

#include "Item.h"
#include "Tuple.h"

#include "TF-IDF.h"
#include "LogisticRegression.h"

/* It is assumed that the json and csv files have proper formatting and appropriate values ,
 * so no extra error checking is done. */

int CalculateBucketSize(char* websitesFolderPath){
    /* Counts the number of files in the folders to estimate a good size for the hash */

    int bucketSize = 0;
    List websiteFolders;
    IF_ERROR_MSG(!GetFolderItems(websitesFolderPath, &websiteFolders), "failed to open/close base folder")
    Node* currWebsiteFolder = websiteFolders.head;
    while(currWebsiteFolder != NULL){
        char websitePath[BUFFER_SIZE];
        sprintf(websitePath,"%s/%s",websitesFolderPath,(char*)(currWebsiteFolder->value));
        List currItems;
        IF_ERROR_MSG(!GetFolderItems(websitePath, &currItems), "failed to open/close website folder")
        bucketSize += currItems.size;

        currWebsiteFolder = currWebsiteFolder->next;
        List_FreeValues(currItems,free);
        List_Destroy(&currItems);
    }
    bucketSize = (int)((float)bucketSize * 1.3f); // NOTE: good size = #keys * 1.3

    List_FreeValues(websiteFolders,free);
    List_Destroy(&websiteFolders);

    return bucketSize;
}

void ParseArgs(int argc, char* argv[], char **websitesFolderPath, char **dataSetWPath, int* bucketSize, 
char** identicalFilePath, char** nonIdenticalFilePath, char** outputFilePath){
    // Get the flags from argv.
    // -f should contain the path to the folder containing the websites folders.
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-f", websitesFolderPath), "Argument -f is missing or has no value")
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-w", dataSetWPath), "Argument -w is missing or has no value")

    // -b is the bucketsize.
    char *bucketSizeStr;
    // If -b is not provided we estimate a good size by counting the number of files.
    // We know each file is equivalent to hash key.
    if(FindArgAfterFlag(argv, argc, "-b", &bucketSizeStr)) {
        IF_ERROR_MSG(!StringToInt(bucketSizeStr, bucketSize), "Bucket Size should be a number")
    }else{
        // We estimated the size based on the files number.
        *bucketSize = CalculateBucketSize(*websitesFolderPath);
        //printf("------------- %d -------------\n",bucketSize);
    }

    //i is the filename for identical pairs to be printed out to
    if(FindArgAfterFlag(argv, argc, "-i", identicalFilePath)) {
        IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-i", identicalFilePath), "Argument -i is missing or has no value")
    }

    //n is the filename for non identical pairs to be printed out to
    if(FindArgAfterFlag(argv, argc, "-n", nonIdenticalFilePath)) {
        IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-n", nonIdenticalFilePath), "Argument -n is missing or has no value")
    }

    //o is the filename for testing output to be printed out to
    if(FindArgAfterFlag(argv, argc, "-o", outputFilePath)) {
        IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-o", outputFilePath), "Argument -o is missing or has no value")
    }
}

void HandleData_X(char* websitesFolderPath,int bucketSize,CliqueGroup* cliqueGroup){
    /* Reads json files inside the given folder path and adds them to a cliqueGroup data structure
     * for further processing. */

    // Open folder from -f (should contain more folder with names of websites)
    List websiteFolders;
    IF_ERROR_MSG(!GetFolderItems(websitesFolderPath, &websiteFolders), "failed to open/close base folder")

    // Open each website folder.
    Node* currWebsiteFolder = websiteFolders.head;
    while(currWebsiteFolder != NULL){
        char websitePath[BUFFER_SIZE];
        sprintf(websitePath,"%s/%s",websitesFolderPath,(char*)(currWebsiteFolder->value));

        // Open each item inside the current website folder.
        List currItems;
        IF_ERROR_MSG(!GetFolderItems(websitePath, &currItems), "failed to open/close website folder")

        /* Create Nodes from the list of Json file names */
        Node* currItem = currItems.head;
        while(currItem != NULL){
            /* The json relative file path
            i.e ../../Datasets/camera_specs/2013_camera_specs/www.walmart.com/767.json */
            char jsonFilePath[2*BUFFER_SIZE];
            sprintf(jsonFilePath,"%s/%s",websitePath,(char*)(currItem->value));

            char itemID[BUFFER_SIZE]; /*format:  website//idNumber */
            sprintf(itemID,"%s//%s",(char*)(currWebsiteFolder->value), (char*)(currItem->value));

            RemoveFileExtension(itemID);

            /* Create item and insert into items list */
            Item* item = Item_Create(itemID, GetJsonPairs(jsonFilePath));
            //item-
            CliqueGroup_Add(cliqueGroup, item->id, (int)strlen(itemID)+1, item);

            currItem = currItem->next;
        }

        currWebsiteFolder = currWebsiteFolder->next;

        List_FreeValues(currItems,free);
        List_Destroy(&currItems);
    }

    List_FreeValues(websiteFolders,free);
    List_Destroy(&websiteFolders);
}

void HandleData_W(char* dataSetWPath,CliqueGroup* cliqueGroup){
    /* Update cliqueGroup with dataSetW.
     * We apply the simple logic that for items a,b,c : if a == b and b == c then a == c.
     * dataSetW contains values of the following format :
     * idA , idB , 0/1
     * 1 = idA and idB are similar items and the cliqueGroup should be updated. */

    FILE* dataSetFile = fopen(dataSetWPath, "r");
    IF_ERROR_MSG(dataSetFile == NULL, "-w file not found")

    List values;
    CSV_GetLine(dataSetFile, &values); // get rid of columns
    List_FreeValues(values, free);
    List_Destroy(&values);
    while(CSV_GetLine(dataSetFile, &values)) {
        char* id1 = (char*)values.head->value;
        char* id2 = (char*)values.head->next->value;
        char* similarityString = (char*)values.head->next->next->value;
        int similarity;
        StringToInt(similarityString,&similarity);

        // If the 2 items are similar we merge the cliques.
        if(similarity == 1) {
            CliqueGroup_Update_Similar(cliqueGroup, id1, (int) strlen(id1) + 1, id2, (int) strlen(id2) + 1);
        }else if(similarity == 0){
            CliqueGroup_Update_NonSimilar(cliqueGroup, id1, (int) strlen(id1) + 1, id2, (int) strlen(id2) + 1);
        }

        List_FreeValues(values, free);
        List_Destroy(&values);
    }

    fclose(dataSetFile);

    //Finalize
    CliqueGroup_Finalize(*cliqueGroup);
}

Hash CreateStopwordHash(char* fileStr){
    FILE* fp = fopen(fileStr, "r");
    assert(fp != NULL);

    Hash stopwords;
    Hash_Init(&stopwords, DEFAULT_HASH_SIZE, RSHash, StringCmp, true);

    char buffer[BUFFER_SIZE];
    while(fscanf(fp, "%s", buffer) != EOF){
        Hash_Add(&stopwords, buffer, strlen(buffer)+1, "-");
    }

    fclose(fp);

    return stopwords;
}

void WordList_Free(void* val){
    List_FreeValues(*(List*)val, free);
    List_Destroy(val);
    free(val);
}

/* Creates all processed items at once because they will
 *be needed many times when creating the models*/
Hash CreateProcessedItems(CliqueGroup cg){
    Hash itemProcessedWords;
    Hash_Init(&itemProcessedWords, cg.hash.bucketSize, cg.hash.hashFunction, cg.hash.cmpFunction, false);

    Hash stopwords = CreateStopwordHash(STOPWORDS_FILE);

    Node* currCliqueNode = cg.cliques.head;
    while (currCliqueNode != NULL){
        Clique* currClique = (Clique*)(currCliqueNode->value);
        List currCliqueList = currClique->similar;

        Node* currIcpNode = currCliqueList.head;
        while(currIcpNode != NULL){
            ItemCliquePair* icp = (ItemCliquePair*)(currIcpNode->value);

            List* itemWords = Item_Preprocess(icp->item, stopwords);

            Hash_Add(&itemProcessedWords, &icp->id, sizeof(icp->id), itemWords);

            currIcpNode = currIcpNode->next;
        }

        currCliqueNode = currCliqueNode->next;
    }

    Hash_Destroy(stopwords);

    return itemProcessedWords;
}

bool UIntCmp(void* value1, void* value2){
    if(*(unsigned int*)value1 == *(unsigned int*)value2){
        return true;
    }else{
        return false;
    }
}

void CreateXY(List items, List pairs, Hash idfDictionary, Hash itemProcessedWords, double*** X, unsigned int*** xIndexes, double** Y){
    //Array of sparse matrices(hashes) with TFIDF values
    Hash* x = CreateVectors(items, idfDictionary, itemProcessedWords);
    
    //Hash with keys = id of icp and value = index in array of TFIDF vectors
    Hash indexes;
    Hash_Init(&indexes, DEFAULT_HASH_SIZE, RSHash, UIntCmp, false);
    
    //Array with TFIDF vectors for each item
    double** xVals = malloc(items.size * sizeof(double*));
    
    int k = 0;
    Node* itemNode = items.head;
    while(itemNode != NULL){
        ItemCliquePair* icp = (ItemCliquePair*)itemNode->value;

        unsigned int* index = malloc(sizeof(unsigned int));
        *index = k;
        Hash_Add(&indexes, &icp->id, sizeof(icp->id), index);
        xVals[k] = TF_IDF_ToArray(x[k], idfDictionary);

        Hash_FreeValues(x[k], free);
        Hash_Destroy(x[k]);

        k++;
        itemNode = itemNode->next;
    }

    free(x);

    //Array of vector indexes in TFIDF array for all pairs
    unsigned int** pairIndexes = malloc(pairs.size * sizeof(unsigned int*));
    Node* pairNode = pairs.head;
    k = 0;
    //Mapping
    while(pairNode != NULL){
        pairIndexes[k] = malloc(2 * sizeof(unsigned int));

        Tuple* pair = (Tuple*)pairNode->value;

        ItemCliquePair* icp1 = (ItemCliquePair*)pair->value1;
        unsigned int* index1 = Hash_GetValue(indexes, &icp1->id, sizeof(icp1->id));
        pairIndexes[k][0] = *index1; 

        ItemCliquePair* icp2 = (ItemCliquePair*)pair->value2;
        unsigned int* index2 = Hash_GetValue(indexes, &icp2->id, sizeof(icp2->id));
        pairIndexes[k][1] = *index2; 

        k++;
        pairNode = pairNode->next;
    }

    double* yVals = CreateY(pairs);

    *Y = yVals;
    *X = xVals;
    *xIndexes = pairIndexes;

    //Cleanup
    Hash_FreeValues(indexes, free);
    Hash_Destroy(indexes);
}

void RedirectFileDescriptorToFile(int fdR, char* filePath, int* fd_new, int* fd_copy){
    *fd_new = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    //redirect printfs to the file
    *fd_copy = dup(fdR);
    dup2(*fd_new, 1);
}

void ResetFileDescriptor(int fdR, int fd_new, int fd_copy){
    //close file
    close(fd_new);
    
    //reset
    dup2(fd_copy, fdR);
    close(fd_copy);
}

int main(int argc, char* argv[]){
    /* --- Arguments --------------------------------------------------------------------------*/

    char *websitesFolderPath , *dataSetWPath, *identicalFilePath, *nonIdenticalFilePath, *outputFilePath;
    int bucketSize;
    ParseArgs(argc, argv, &websitesFolderPath, &dataSetWPath, &bucketSize, &identicalFilePath, &nonIdenticalFilePath, &outputFilePath);

    /* --- Reads Json files and adds them to the clique ---------------------------------------*/

    CliqueGroup cliqueGroup;
    CliqueGroup_Init(&cliqueGroup, bucketSize, RSHash, StringCmp);
    HandleData_X(websitesFolderPath,bucketSize,&cliqueGroup);

    /* --- Reads CSV files and updates the cliqueGroups ---------------------------------------*/

    HandleData_W(dataSetWPath, &cliqueGroup);

    /* --- Print results ----------------------------------------------------------------------*/
    
    printf("\n");

    //Get Identical Pairs
    List trainingPairs = CliqueGroup_GetIdenticalPairs(&cliqueGroup);
    printf("%d identical pairs found, printed in %s\n\n", trainingPairs.size, identicalFilePath);
    
    //Redirect stdout to the fd of the identicalFilePath
    int fd_copy, fd_new;
    RedirectFileDescriptorToFile(1, identicalFilePath, &fd_new, &fd_copy);

    //Print
    CliqueGroup_PrintPairs(trainingPairs, Item_Print);

    //Reset stdout
    ResetFileDescriptor(1, fd_new, fd_copy);

    //Get Non Identical Pairs
    List nonIdenticalPairs = CliqueGroup_GetNonIdenticalPairs(&cliqueGroup);
    printf("%d non identical pairs found, printed in %s\n\n", nonIdenticalPairs.size, nonIdenticalFilePath);
    
    //Redirect stdout to the fd of the nonIdenticalFilePath
    RedirectFileDescriptorToFile(1, nonIdenticalFilePath, &fd_new, &fd_copy);

    //Print
    CliqueGroup_PrintPairs(nonIdenticalPairs, Item_Print);

    //Reset stdout
    ResetFileDescriptor(1, fd_new, fd_copy);

    // Join lists for later training.
    List_Join(&trainingPairs, &nonIdenticalPairs);
    // large : 299395 , medium : 43074

    //60-40
    double trainingPercentage = 0.6, testingPercentage = 0.2, validationPercentage = 0.2;

    //TODO: randomize trainingPairs before splitting
    List testingPairs;
    if (List_Split(&trainingPairs, &testingPairs, trainingPercentage) == false){
        printf("Can't split dataset for testing\n");
        exit(1);
    }else{
        printf("Splitted for testing\n\n");
    }

    List validationPairs;
    if (List_Split(&testingPairs, &validationPairs, testingPercentage + trainingPercentage/2) == false){
        printf("Can't split dataset for validation\n");
        exit(1);
    }else{
        printf("Splitted for validation\n\n");
    }

    printf("Training size: %d pairs (%.2f%%)\nTesting size: %d pairs (%.2f%%)\nValidation size: %d pairs (%.2f%%)\n\n", 
    trainingPairs.size, trainingPercentage*100, validationPairs.size, testingPercentage*100, validationPairs.size, validationPercentage*100);

    //Get all items in a list to use later
    List items = CliqueGroup_GetAllItems(cliqueGroup);
    
    /* --- Create processed words for items ---------------------------------------------------*/

    Hash itemProcessedWords = CreateProcessedItems(cliqueGroup);
    printf("Created Processed Words\n\n");

    Hash idfDictionary = IDF_Calculate(items, itemProcessedWords, 1000); //Create Dictionary based on items list
    printf("Created and Trimmed Dictionary based on average TFIDF\n\n");

    double** xValsTraining;
    unsigned int** xIndexesTraining;
    double* yValsTraining;

    
    unsigned int width = 2 * idfDictionary.keyValuePairs.size;
    unsigned int height = trainingPairs.size;

    CreateXY(items, trainingPairs, idfDictionary, itemProcessedWords, &xValsTraining, &xIndexesTraining, &yValsTraining);
    printf("Created X Y Datasets for training\n\n");

    //Training
    LogisticRegression model;
    LogisticRegression_Init(&model, 0, xValsTraining, xIndexesTraining, yValsTraining, width, height, items.size);
    LogisticRegression_Train(&model, 0.001, 1);

    printf("\nTraining completed\n");

    //Testing Datasets
    double** xValsTesting;
    unsigned int** xIndexesTesting;
    double* yValsTesting;

    CreateXY(items, testingPairs, idfDictionary, itemProcessedWords, &xValsTesting, &xIndexesTesting, &yValsTesting);
    printf("Created X Y Datasets for testing\n\n");

    //Start testing
    printf("Starting Tests...");

    //Redirect stdout to the fd of the outputFilePath
    RedirectFileDescriptorToFile(1, outputFilePath, &fd_new, &fd_copy);

    int counter0 = 0;
    int counter1 = 0;
    for (int i = 0; i < testingPairs.size; i++) {
        double* leftVector = xValsTesting[xIndexesTesting[i][0]];
        double* rightVector = xValsTesting[xIndexesTesting[i][1]];
        
        double prediction = LogisticRegression_Predict(&model, leftVector, rightVector);
        if(fabs(yValsTesting[i] - prediction) < 0.1) {
            if(yValsTesting[i] == 0){
                counter0++;
            }else{
                counter1++;
            }
        }
        printf("Prediction : %f Real value : %f\n", prediction, yValsTraining[i]);
    }

    printf("\nGeneral Pair Accuracy : %d / %d\n", counter0 + counter1 , testingPairs.size);
    printf("%d Identical Pairs accurate\n%d Non Identical pairs accurate\n",counter1,counter0);
    printf("\n");

    //Reset stdout
    ResetFileDescriptor(1, fd_new, fd_copy);
    printf("Printed prediction results in %s\n\n", outputFilePath);

    //TODO: we dont know how many identical and non identical pairs there were
    //printf("Identical Pair Accuracy : %d / %d\n", counter1 , testingPairs.size);
    //printf("Non Identical Pair Accuracy : %d / %d\n", counter0 , testingPairs.size);
    printf("General Pair Accuracy : %d / %d\n", counter0 + counter1 , testingPairs.size);
    printf("%d Identical Pairs accurate\n%d Non Identical pairs accurate\n",counter1,counter0);
    printf("\n");

    /* --- Clean up ---------------------------------------------------------------------------*/
    
    printf("Cleaning up...\n\n");


    List_FreeValues(trainingPairs, Tuple_Free);
    List_Destroy(&trainingPairs);

    LogisticRegression_Destroy(model);

    free(yValsTesting);
    for(int i = 0; i < items.size; i++){
        free(xValsTesting[i]);
    }
    for(int i = 0; i < testingPairs.size; i++){
        free(xIndexesTesting[i]);
    }
    free(xValsTesting);
    free(xIndexesTesting);

    List_Destroy(&items);
    
    List_FreeValues(testingPairs, Tuple_Free);
    List_Destroy(&testingPairs);

    List_FreeValues(validationPairs, Tuple_Free);
    List_Destroy(&validationPairs);

    Hash_FreeValues(idfDictionary, free);
    Hash_Destroy(idfDictionary);

    Hash_FreeValues(itemProcessedWords, WordList_Free);
    Hash_Destroy(itemProcessedWords);
    
    CliqueGroup_FreeValues(cliqueGroup, Item_Free);
    CliqueGroup_Destroy(cliqueGroup);

    printf("Exiting...\n");

    return 0;
}