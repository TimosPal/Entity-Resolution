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
#include "JobScheduler.h"

JobScheduler jobScheduler; // Global so it can be accessed from everywhere

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
char** identicalFilePath, char** nonIdenticalFilePath, char** outputFilePath, int* vocabSize, int* epochs, double* maxAccuracyDiff, double* learningRate, int* threadCount){
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
    }

    // -v is the vocabSize
    char *vocabSizeStr;
    // If -v is not provided we give it a default value.
    if(FindArgAfterFlag(argv, argc, "-v", &vocabSizeStr)) {
        IF_ERROR_MSG(!StringToInt(vocabSizeStr, vocabSize), "Vocabulary Size should be a number")
    }else{
        // Give it a default value
        *vocabSize = VOCAB_SIZE;
    }

    // -t is the thread count
    char *threadCountStr;
    // If -t is not provided we give it a default value.
    if(FindArgAfterFlag(argv, argc, "-t", &threadCountStr)) {
        IF_ERROR_MSG(!StringToInt(threadCountStr, threadCount), "Thread Count should be a number")
    }else{
        // Give it a default value
        *threadCount = WORKERS;
    }

    // -e is the number of epochs
    char *epochsStr;
    // If -e is not provided we give it a default value.
    if(FindArgAfterFlag(argv, argc, "-e", &epochsStr)) {
        IF_ERROR_MSG(!StringToInt(epochsStr, epochs), "Epochs should be a number")
    }else{
        // Give it a default value
        *epochs = EPOCHS;
    }

    // -d is the max accuracy difference for testing
    char *maxAccuracyDiffStr;
    // If -d is not provided we give it a default value.
    if(FindArgAfterFlag(argv, argc, "-d", &maxAccuracyDiffStr)) {
        IF_ERROR_MSG(!StringToDouble(maxAccuracyDiffStr, maxAccuracyDiff), "Accuracy Difference Percentage should be a number")
    }else{
        // Give it a default value
        *maxAccuracyDiff = MAX_ACCURACY_DIFF;
    }

    // -r is the learning rate for model training
    char *learningRateStr;
    // If -r is not provided we give it a default value.
    if(FindArgAfterFlag(argv, argc, "-r", &learningRateStr)) {
        IF_ERROR_MSG(!StringToDouble(learningRateStr, learningRate), "Learning rate should be a number")
    }else{
        // Give it a default value
        *learningRate = LEARNING_RATE;
    }

    //i is the filename for identical pairs to be printed out to
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-i", identicalFilePath), "Argument -i is missing or has no value")

    //n is the filename for non identical pairs to be printed out to
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-n", nonIdenticalFilePath), "Argument -n is missing or has no value")

    //o is the filename for testing output to be printed out to
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-o", outputFilePath), "Argument -o is missing or has no value")
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

void HandleData_W(char* dataSetWPath, CliqueGroup* cliqueGroup, List* testingPairs, List* validationPairs){
    /* Update cliqueGroup with dataSetW.
     * We apply the simple logic that for items a,b,c : if a == b and b == c then a == c.
     * dataSetW contains values of the following format :
     * idA , idB , 0/1
     * 1 = idA and idB are similar items and the cliqueGroup should be updated. */

    FILE* dataSetFile = fopen(dataSetWPath, "r");
    IF_ERROR_MSG(dataSetFile == NULL, "-w file not found")

    List pairs;
    List_Init(&pairs);
    List_Init(testingPairs);
    List_Init(validationPairs);

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

        //Alloc tuple
        Tuple* pair = malloc(sizeof(Tuple));
        ItemCliquePair** icpPair = malloc(2*sizeof(ItemCliquePair*));
        bool* similarityPtr = malloc(sizeof(bool));

        //Values of tuple
        icpPair[0] = Hash_GetValue(cliqueGroup->hash, id1, strlen(id1)+1);
        icpPair[1] = Hash_GetValue(cliqueGroup->hash, id2, strlen(id2)+1);
        
        *similarityPtr = similarity;

        //Set Tuple
        pair->value1 = icpPair;
        pair->value2 = similarityPtr;

        //Append Tuple to Pairs list
        List_Append(&pairs, pair);


        List_FreeValues(values, free);
        List_Destroy(&values);
    }

    fclose(dataSetFile);

    printf("Pairs in file are %d\n", pairs.size);
    //Shuffle and Split all pairs
    List_Shuffle(&pairs);

    double trainingPercentage = 0.6, testingPercentage = 0.2, validationPercentage = 0.2;

    IF_ERROR_MSG(!List_Split(&pairs, testingPairs, trainingPercentage), "Can't split dataset for testing")
    printf("Split for testing\n");

    IF_ERROR_MSG(!List_Split(testingPairs, validationPairs, testingPercentage + trainingPercentage/2), "Can't split dataset for validation")
    printf("Split for validation\n\n");

    printf("Training size: %d pairs (%.2f%%)\nTesting size: %d pairs (%.2f%%)\nValidation size: %d pairs (%.2f%%)\n\n",
    pairs.size, trainingPercentage*100, validationPairs->size, testingPercentage*100, validationPairs->size, validationPercentage*100);

    Node* pairNode = pairs.head;
    while(pairNode != NULL){
        Tuple* pair = (Tuple*)pairNode->value;
        ItemCliquePair** icpPair = pair->value1;
        bool* similarityPtr = (bool*)pair->value2;

        Item* item1 = icpPair[0]->item;
        Item* item2 = icpPair[1]->item;

        // If the 2 items are similar we merge the cliques.
        if(*similarityPtr) {
            CliqueGroup_Update_Similar(cliqueGroup, item1->id, (int) strlen(item1->id) + 1, item2->id, (int) strlen(item2->id) + 1);
        }else{
            CliqueGroup_Update_NonSimilar(cliqueGroup, item1->id, (int) strlen(item1->id) + 1, item2->id, (int) strlen(item2->id) + 1);
        }

        free(pair->value1);
        free(pair->value2);
        free(pair);

        pairNode = pairNode->next;
    }
    
    //Finalize
    CliqueGroup_Finalize(*cliqueGroup);

    //Cleanup
    List_Destroy(&pairs);
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

bool IcpCmp(void* value1, void* value2){
    ItemCliquePair* icp1 = value1;
    ItemCliquePair* icp2 = value2;

    return (icp1->id == icp2->id);
}

int PairChance_Cmp(const void* value1, const void* value2){
    Tuple* tuple1 = *(Tuple**)value1;
    Tuple* tuple2 = *(Tuple**)value2;

    double acc1 = *(double*)(tuple1->value2);
    double acc2 = *(double*)(tuple2->value2);
    
    if(1 - acc1 < acc1){
        acc1 = 1 - acc1;
    }

    if(1 - acc2 < acc2){
        acc2 = 1 - acc2;
    }

    if (acc1 < acc2){
        return -1;
    }else if(acc1 > acc2){
        return 1;
    }else{
        return 0;
    }
}

void CreateXVals(List items, Hash idfDictionary, Hash itemProcessedWords, Hash** XVals, Hash* icpToIndex, Hash* indexToIcp){
    //Array of sparse matrices(hashes) with TFIDF values
    Hash* x = CreateVectors(items, idfDictionary, itemProcessedWords);
    
    //Hash with keys = id of icp and value = index in array of TFIDF vectors
    Hash_Init(icpToIndex, DEFAULT_HASH_SIZE, RSHash, UIntCmp, false);
    Hash_Init(indexToIcp, DEFAULT_HASH_SIZE, RSHash, IcpCmp, false);
    
    //Array with TFIDF vectors for each item
    Hash* xVals = malloc(items.size * sizeof(Hash));
    
    unsigned int k = 0;
    Node* itemNode = items.head;
    while(itemNode != NULL){
        ItemCliquePair* icp = (ItemCliquePair*)itemNode->value;

        unsigned int* index = malloc(sizeof(unsigned int));
        *index = k;
        //add to icpToIndex Hash
        Hash_Add(icpToIndex, &icp->id, sizeof(icp->id), index);
        //add to indexToIcp Hash
        Hash_Add(indexToIcp, index, sizeof(unsigned int), icp);
        xVals[k] = TF_IDF_ToIndexHash(x[k], idfDictionary);

        Hash_FreeValues(x[k], free);
        Hash_Destroy(x[k]);

        k++;
        itemNode = itemNode->next;
    }

    free(x);

    *XVals = xVals;
}

double* CreateY(List pairs){
    unsigned int height = (unsigned int)pairs.size;
    double* results = malloc(height * sizeof(double));

    int index = 0;
    Node* currPairNode = pairs.head;
    while (currPairNode != NULL){
        Tuple* currTuple = currPairNode->value;

        //Y
        bool* similarityPtr = currTuple->value2;
        results[index] = (*similarityPtr == true) ? 1.0 : 0.0;

        index++;
        currPairNode = currPairNode->next;
    }

    return results;
}

void CreateX(List items, List pairs, Hash idfDictionary, Hash itemProcessedWords, Hash indexes, unsigned int*** xIndexes){
    //Array of vector indexes in TFIDF array for all pairs
    unsigned int** pairIndexes = malloc(pairs.size * sizeof(unsigned int*));
    Node* pairNode = pairs.head;
    int k = 0;
    //Mapping
    while(pairNode != NULL){
        pairIndexes[k] = malloc(2 * sizeof(unsigned int));

        Tuple* pair = (Tuple*)pairNode->value;

        ItemCliquePair** icpPair = pair->value1;
        ItemCliquePair* icp1 = icpPair[0];
        unsigned int* index1 = Hash_GetValue(indexes, &icp1->id, sizeof(icp1->id));
        pairIndexes[k][0] = *index1; 

        ItemCliquePair* icp2 = icpPair[1];
        unsigned int* index2 = Hash_GetValue(indexes, &icp2->id, sizeof(icp2->id));
        pairIndexes[k][1] = *index2; 

        //sort the 2 indexes
        if (pairIndexes[k][0] > pairIndexes[k][1]){
            unsigned int temp = pairIndexes[k][0];
            pairIndexes[k][0] = pairIndexes[k][1];
            pairIndexes[k][1] = temp;
        }

        k++;
        pairNode = pairNode->next;
    }

    *xIndexes = pairIndexes;
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

typedef struct Training_Pack{
    Hash* icpToIndex;
    Hash* indexToIcp;
    List* trainingPairs;
    List* testingPairs;

    double learningRate;
    int epochs;
} Training_Pack;

typedef struct Item_Pack{
    Hash* idfDictionary;
    Hash* itemProcessedWords;
    List* items;
} Item_Pack;

/* get predictions of the pairs given and return list with tuples(prediction, real value) */
void* PredictPairs(void** args){
    int* fromIndex = args[0];
    int* toIndex = args[1];
    unsigned int** xIndexesTesting = args[2];
    Hash* xVals = args[3];
    double *yVals = args[4];
    LogisticRegression* model = args[5];
    
    List* predictions = malloc(sizeof(List));
    List_Init(predictions);

    for(int i = *fromIndex; i < *toIndex; i++){
        
        Hash leftVector = xVals[xIndexesTesting[i][0]]; //Hash1
        Hash rightVector = xVals[xIndexesTesting[i][1]]; //Hash2

        double prediction = LogisticRegression_Predict(model, &leftVector, &rightVector);

        //Add pair and predictionError tuple to list
        Tuple* pairPredictionErrorTuple = malloc(sizeof(Tuple));
        Tuple_Init(pairPredictionErrorTuple, &prediction, sizeof(double), &yVals[i], sizeof(double));
        
        List_Append(predictions, pairPredictionErrorTuple); 
    }

    return predictions;
}

void* GetThresholdPair(void** args){
    int* fromIndex = args[0];
    int* toIndex = args[1];
    unsigned int** xIndexesTesting = args[2];
    Hash* xVals = args[3];
    LogisticRegression* model = args[4];
    double* threshold = args[5];
    
    List* acceptedPairs = malloc(sizeof(List));
    List_Init(acceptedPairs);

    //check if this pair has already been added to training set before
    for(int i = *fromIndex; i < *toIndex; i++){
        if(xIndexesTesting[i][0] == -1 || xIndexesTesting[i][1] == -1){
            continue;
        }
        
        Hash leftVector = xVals[xIndexesTesting[i][0]]; //Hash1
        Hash rightVector = xVals[xIndexesTesting[i][1]]; //Hash2

        double prediction = LogisticRegression_Predict(model, &leftVector, &rightVector);

        //chance is prediction
        double predictionError = prediction;
        //chance = 1 - error if it is closer to 1
        if (1 - predictionError < predictionError){
            predictionError = 1 - predictionError;
        }
        //if error is under threshold
        if(predictionError < *threshold) {
            //Add pair and predictionError tuple to list
            Tuple* pairPredictionErrorTuple = malloc(sizeof(Tuple));
            //we pass the prediction and not the prediction error in order to know how to retrain
            Tuple_Init(pairPredictionErrorTuple, &i, sizeof(int), &prediction, sizeof(double));
            List_Append(acceptedPairs, pairPredictionErrorTuple); 
        }
    }

    return acceptedPairs;
}

/* Get n random pairs for items in the items list */
List GetRetrainingPairs(List* items, int n){
    List retrainingPairs;
    List_Init(&retrainingPairs);

    for (int i = 0; i < n; i++){
        int index1 = rand() % items->size;
        int index2 = rand() % items->size;

        Node* item1Node = List_GetNode(*items, index1);
        Node* item2Node = List_GetNode(*items, index2);

        ItemCliquePair* icp1 = item1Node->value;
        ItemCliquePair* icp2 = item2Node->value;

        //Alloc tuple
        Tuple* tuple = malloc(sizeof(Tuple));

        //malloc values
        ItemCliquePair** icpPair = malloc(2*sizeof(ItemCliquePair*));
        int* similarityPtr = malloc(sizeof(int));

        //Set values
        icpPair[0] = icp1;
        icpPair[1] = icp2;
        *similarityPtr = -1;

        //Set the Tuple
        tuple->value1 = icpPair;
        tuple->value2 = similarityPtr;

        List_Append(&retrainingPairs, tuple);
    }

    return retrainingPairs;
}

void TestModelOn(LogisticRegression* model, Item_Pack* itemPack, List testingPairs, Hash* indexToIcp, Hash* xVals, char* outputFilePath, double maxAccuracyDiff){
    unsigned int **xIndexesTesting;
    double *yVals;
    CreateX(*itemPack->items, testingPairs, *itemPack->idfDictionary, *itemPack->itemProcessedWords, *indexToIcp, &xIndexesTesting);
    yVals = CreateY(testingPairs);

    int batchSize = testingPairs.size / jobScheduler.numberOfThreads;
    int excessBatchSize = testingPairs.size % jobScheduler.numberOfThreads;

    for (int i = 0; i < jobScheduler.numberOfThreads; i++) {
        void** args = malloc(6 * sizeof(void*));
        int* fromIndex = malloc(sizeof(int));
        int* toIndex = malloc(sizeof(int));
        *fromIndex = i * batchSize;
        *toIndex = i * batchSize + batchSize;
        if (i == jobScheduler.numberOfThreads - 1){
            (*toIndex)+= excessBatchSize;
        }
        

        // Thread args.
        args[0] = fromIndex;
        args[1] = toIndex;
        args[2] = xIndexesTesting;
        args[3] = xVals;
        args[4] = yVals;
        args[5] = model;

        // Create job.
        Job* newJob = malloc(sizeof(Job));
        Job_Init(newJob, PredictPairs, Tuple_Free, args);
        JobScheduler_AddJob(&jobScheduler, newJob);
    }

    // Sync threads.
    JobScheduler_WaitForJobs(&jobScheduler, jobScheduler.numberOfThreads);
    
    // Append results to the accaptedPairs list.
    List testingPredictions;
    List_Init(&testingPredictions);
    while(jobScheduler.results.head != NULL) {
        Job* currJob = Queue_Pop(&jobScheduler.results);

        free(currJob->taskArgs[0]);
        free(currJob->taskArgs[1]);
        free(currJob->taskArgs);

        // If the value was accepted it , append it to the list
        // and increment the counters accordingly.
        if(currJob->result != NULL){
            List_Join(&testingPredictions, currJob->result);
            free(currJob->result);
        }

        free(currJob);
    }

    //Redirect stdout to the fd of the outputFilePath
    int fd_copy, fd_new;
    RedirectFileDescriptorToFile(1, outputFilePath, &fd_new, &fd_copy);

    //Now testingPredictions contains all predictions for the testing set and all real values in tuples
    int testingCounter0 = 0, testingCounter1 = 0;
    int testingReal1 = 0, testingReal0 = 0;

    Node* predictionNode = testingPredictions.head;
    while(predictionNode != NULL){
        Tuple* prediction_realValue_tuple = predictionNode->value;
        double prediction = *(double*)prediction_realValue_tuple->value1;
        double realValue = *(double*)prediction_realValue_tuple->value2;

        if(realValue == 1.0){
            testingReal1++;
        }else{
            testingReal0++;
        }

        if(fabs(prediction - realValue) < maxAccuracyDiff){
            if (realValue == 1.0){
                testingCounter1++;
            }else{
                testingCounter0++;
            }
        }
        printf("Prediction : %f    Real Value : %f\n", prediction, realValue);

        predictionNode = predictionNode->next;
    }
    double accuracyPercentage = ((double)testingCounter1 + testingCounter0) / testingPairs.size * 100;
    double identicalPercentage = (double)(testingCounter1) / testingReal1 * 100;
    double nonIdenticalPercentage = (double)(testingCounter0) / testingReal0 * 100;
    printf("Identical Pair Accuracy : %d / %d (%f%%)\n", testingCounter1 , testingReal1, identicalPercentage);
    printf("Non Identical Pair Accuracy : %d / %d (%f%%)\n", testingCounter0 , testingReal0, nonIdenticalPercentage);
    printf("General Pair Accuracy : %d / %d (%f%%)\n", testingCounter0 + testingCounter1 , testingPairs.size, accuracyPercentage );
    printf("\n");

    //Reset stdout
    ResetFileDescriptor(1, fd_new, fd_copy);

    printf("Identical Pair Accuracy : %d / %d (%f%%)\n", testingCounter1 , testingReal1, identicalPercentage);
    printf("Non Identical Pair Accuracy : %d / %d (%f%%)\n", testingCounter0 , testingReal0, nonIdenticalPercentage);
    printf("General Pair Accuracy : %d / %d (%f%%)\n", testingCounter0 + testingCounter1 , testingPairs.size, accuracyPercentage );
    printf("\n");

    //Cleanup
    free(yVals);
    for(int i = 0; i < testingPairs.size; i++){
        free(xIndexesTesting[i]);
    }
    free(xIndexesTesting);

    List_FreeValues(testingPredictions, Tuple_Free);
    List_Destroy(&testingPredictions);
}

void DynamicLearning(CliqueGroup* cliqueGroup, LogisticRegression* model, Training_Pack* trainingPack, Item_Pack* itemPack, Hash* xVals){
    unsigned int width = 2 * itemPack->idfDictionary->keyValuePairs.size;

    printf("Created X Y Datasets for training\n");
    
    //Testing Datasets
    unsigned int** xIndexesTesting;
    
    CreateX(*itemPack->items,
            *trainingPack->testingPairs,
            *itemPack->idfDictionary,
            *itemPack->itemProcessedWords,
            *trainingPack->icpToIndex,
            &xIndexesTesting);


    printf("Created X Dataset for retraining predictions\n\n");
    
    double threshold = THRESHOLD;
    double stepValue = STEP_VALUE;
    int retrainCounter = 0;

    LogisticRegression_Init(model, 0, xVals, width, itemPack->items->size);

    int testingSize = trainingPack->testingPairs->size;
    while(threshold < 0.5 && retrainCounter != TRAINING_STEPS){ // also stop if it has retrained TRAINING_STEPS times
        unsigned int height = trainingPack->trainingPairs->size;
        unsigned int** xIndexesTraining;
        double* yValsTraining;

        CreateX(*itemPack->items,
                 *trainingPack->trainingPairs,
                 *itemPack->idfDictionary,
                 *itemPack->itemProcessedWords,
                 *trainingPack->icpToIndex,
                 &xIndexesTraining);

        yValsTraining = CreateY(*trainingPack->trainingPairs);

        //Destroy the model to free the weights before next init
        LogisticRegression_Destroy(*model);
        
        printf("Starting training #%d...\n", retrainCounter);
        LogisticRegression_Init(model, 0, xVals, width, itemPack->items->size);
        LogisticRegression_Train(model,xIndexesTraining,yValsTraining,height, trainingPack->learningRate, trainingPack->epochs);
        //Start predicting
        printf("Predicting #%d...\n", retrainCounter);

        List acceptedPairs;
        List_Init(&acceptedPairs);

        int batchSize = trainingPack->testingPairs->size / jobScheduler.numberOfThreads;
        int excessBatchSize = trainingPack->testingPairs->size % jobScheduler.numberOfThreads;

        for (int i = 0; i < jobScheduler.numberOfThreads; i++) {
            void** args = malloc(6 * sizeof(void*));
            int* fromIndex = malloc(sizeof(int));
            int* toIndex = malloc(sizeof(int));
            *fromIndex = i * batchSize;
            *toIndex = i * batchSize + batchSize;
            if (i == jobScheduler.numberOfThreads - 1) 
                (*toIndex)+= excessBatchSize;

            // Thread args.
            args[0] = fromIndex;
            args[1] = toIndex;
            args[2] = xIndexesTesting;
            args[3] = xVals;
            args[4] = model;
            args[5] = &threshold;

            // Create job.
            Job* newJob = malloc(sizeof(Job));
            Job_Init(newJob, GetThresholdPair, Tuple_Free, args);
            JobScheduler_AddJob(&jobScheduler, newJob);
        }

        // Sync threads.
        JobScheduler_WaitForJobs(&jobScheduler, jobScheduler.numberOfThreads);
        
        // Append results to the accaptedPairs list.
        while(jobScheduler.results.head != NULL) {
            Job* currJob = Queue_Pop(&jobScheduler.results);

            free(currJob->taskArgs[0]);
            free(currJob->taskArgs[1]);
            free(currJob->taskArgs);

            // If the value was accepted it , append it to the list
            // and increment the counters accordingly.
            if(currJob->result != NULL){
                List_Join(&acceptedPairs, currJob->result);
                free(currJob->result);
            }

            free(currJob);
        }
        
        //Tuple List to array
        Tuple** acceptedPairsArray = (Tuple**)List_ToArray(acceptedPairs);
        
        //Sort array based on accuracy, we translate the prediction into the prediction's error before comparing in PairChance_Cmp
        qsort(acceptedPairsArray, acceptedPairs.size, sizeof(Tuple*), PairChance_Cmp);
        
        //Insert to cliquegroup from higher to lower accuracy (and check if can be inserted)
        for (int i = 0; i < acceptedPairs.size; i++){
            unsigned int index1 = xIndexesTesting[*(unsigned int*)acceptedPairsArray[i]->value1][0];
            unsigned int index2 = xIndexesTesting[*(unsigned int*)acceptedPairsArray[i]->value1][1];
            double prediction = *(double*)(acceptedPairsArray[i]->value2);

            ItemCliquePair* icp1 = Hash_GetValue(*trainingPack->indexToIcp, &index1, sizeof(unsigned int));
            ItemCliquePair* icp2 = Hash_GetValue(*trainingPack->indexToIcp, &index2, sizeof(unsigned int));
            bool isEqual = (1 - prediction < 0.5) ? true : false;

            //if pair is valid into cliqueGroup
            Item* item1 = icp1->item;
            Item* item2 = icp2->item;
  
            //if the pair is valid, then update the cliqueGroup with it
            if (CliqueGroup_PairIsValid(icp1, icp2, isEqual)){
                if(isEqual){
                    CliqueGroup_Update_Similar(cliqueGroup, item1->id, strlen(item1->id)+1, item2->id, strlen(item2->id)+1);
                }else{
                    CliqueGroup_Update_NonSimilar(cliqueGroup, item1->id, strlen(item1->id)+1, item2->id, strlen(item2->id)+1);
                }
            }

            //"remove" these testing vectors
            testingSize--;
            xIndexesTesting[*(unsigned int*)acceptedPairsArray[i]->value1][0] = -1;
            xIndexesTesting[*(unsigned int*)acceptedPairsArray[i]->value1][1] = -1;
        }


        //Finalize cliqueGroup
        CliqueGroup_Finalize(*cliqueGroup);

        //Cleanup
        free(yValsTraining);
        for(int i = 0; i < trainingPack->trainingPairs->size; i++){
            free(xIndexesTraining[i]);
        }
        free(xIndexesTraining);

        //Get Pairs that will be trained in the next loop
        List_FreeValues(*trainingPack->trainingPairs, Tuple_Free);
        List_Destroy(trainingPack->trainingPairs);
        *trainingPack->trainingPairs = CliqueGroup_GetIdenticalPairs(cliqueGroup);
        List nonIdenticalPairs = CliqueGroup_GetNonIdenticalPairs(cliqueGroup);
        List_Join(trainingPack->trainingPairs, &nonIdenticalPairs);

        //More Cleanup

        List_FreeValues(acceptedPairs, Tuple_Free);
        List_Destroy(&acceptedPairs);
        free(acceptedPairsArray);

        //increment threshold and retrainCounter
        retrainCounter++;
        threshold += stepValue;
    }

    //Cleanup
    for(int i = 0; i < trainingPack->testingPairs->size; i++){
        free(xIndexesTesting[i]);
    }
    free(xIndexesTesting);
}

int main(int argc, char* argv[]){
    /* --- Arguments --------------------------------------------------------------------------*/

    char *websitesFolderPath , *dataSetWPath, *identicalFilePath, *nonIdenticalFilePath, *outputFilePath;
    int bucketSize, vocabSize, epochs, threadCount;
    double maxAccuracyDiff, learningRate;
    ParseArgs(argc, argv, &websitesFolderPath, &dataSetWPath, &bucketSize, &identicalFilePath, &nonIdenticalFilePath, &outputFilePath, &vocabSize, &epochs, &maxAccuracyDiff, &learningRate, &threadCount);

    /* --- Init the job scheduler for multi threading wherever needed ------------------------*/

    // NOTE: jobScheduler is global.
    JobScheduler_Init(&jobScheduler, threadCount);

    /* --- Reads Json files and adds them to the clique ---------------------------------------*/


    CliqueGroup cliqueGroup;
    CliqueGroup_Init(&cliqueGroup, bucketSize, RSHash, StringCmp);
    HandleData_X(websitesFolderPath,bucketSize,&cliqueGroup);

    /* --- Reads CSV files and updates the cliqueGroup ----------------------------------------*/
    List testingPairs;
    List validationPairs;
    HandleData_W(dataSetWPath, &cliqueGroup, &testingPairs, &validationPairs);

    /* --- Print results ----------------------------------------------------------------------*/
    
    printf("\n");

    //Get Identical Pairs
    List trainingPairs = CliqueGroup_GetIdenticalPairs(&cliqueGroup);
    printf("%d identical pairs found, printed in %s\n", trainingPairs.size, identicalFilePath);
    
    //Redirect stdout to the fd of the identicalFilePath
    int fd_copy, fd_new;
    RedirectFileDescriptorToFile(1, identicalFilePath, &fd_new, &fd_copy);
    CliqueGroup_PrintPairs(trainingPairs, Item_Print);
    ResetFileDescriptor(1, fd_new, fd_copy);

    //Get Non Identical Pairs
    List nonIdenticalPairs = CliqueGroup_GetNonIdenticalPairs(&cliqueGroup);
    printf("%d non identical pairs found, printed in %s\n\n", nonIdenticalPairs.size, nonIdenticalFilePath);
    
    printf("New training size after transitivity: %d pairs\n\n", nonIdenticalPairs.size + trainingPairs.size);
    //Redirect stdout to the fd of the nonIdenticalFilePath
    RedirectFileDescriptorToFile(1, nonIdenticalFilePath, &fd_new, &fd_copy);
    CliqueGroup_PrintPairs(nonIdenticalPairs, Item_Print);
    ResetFileDescriptor(1, fd_new, fd_copy);

    // Join lists for later training.
    List_Join(&trainingPairs, &nonIdenticalPairs);

    //Get all items in a list to use later
    List items = CliqueGroup_GetAllItems(cliqueGroup);
    
    /* --- Create processed words for items ---------------------------------------------------*/

    Hash itemProcessedWords = CreateProcessedItems(cliqueGroup);
    printf("Created Processed Words\n");
    Hash idfDictionary = IDF_Calculate(items, itemProcessedWords, vocabSize); //Create Dictionary based on items list
    printf("Created and Trimmed Dictionary based on average TFIDF\n");


    Hash* xVals;
    Hash icpToIndex, indexToIcp;
    CreateXVals(items, idfDictionary, itemProcessedWords, &xVals, &icpToIndex, &indexToIcp);
    printf("Created TFIDF Vectors\n");

    LogisticRegression model;

    //Get pairs for which the model will be retrained
    List retrainingPairs = GetRetrainingPairs(&items, RETRAINING_TESTING_PAIRS);
    printf("Created X Dataset for retraining predictions\n");

    //Training
    //This pack contains the testing Pairs for which the model will be retrained
    Training_Pack trainingPack = {.epochs = epochs,
                                  .learningRate = learningRate,
                                  .testingPairs = &retrainingPairs,
                                  .trainingPairs = &trainingPairs,
                                  .icpToIndex = &icpToIndex,
                                  .indexToIcp = &indexToIcp,
                                  };

    Item_Pack itemPack = {.items = &items,
                          .idfDictionary = &idfDictionary,
                          .itemProcessedWords = &itemProcessedWords
                         };

    DynamicLearning(&cliqueGroup, &model, &trainingPack, &itemPack, xVals);

    //Start testing
    printf("Getting Predictions on test set...\n");
    TestModelOn(&model, &itemPack, testingPairs, &indexToIcp, xVals, outputFilePath, maxAccuracyDiff);

    printf("Getting Predictions on validation set...\n");
    TestModelOn(&model, &itemPack, validationPairs, &indexToIcp, xVals, outputFilePath, maxAccuracyDiff);
    //Redirect stdout to the fd of the outputFilePath
    ///RedirectFileDescriptorToFile(1, outputFilePath, &fd_new, &fd_copy);
    //Reset stdout
    //ResetFileDescriptor(1, fd_new, fd_copy);
    //printf("Printed prediction results in %s\n\n", outputFilePath);
    
    //NOTE: PREDICTION PRINT FOR LATER USE
    //printf("Prediction : %f Real value : %f\n", prediction, yValsTesting[i]);

    //TODO: we dont know how many identical and non identical pairs there were
    //printf("Identical Pair Accuracy : %d / %d\n", counter1 , testingPairs.size);
    //printf("Non Identical Pair Accuracy : %d / %d\n", counter0 , testingPairs.size);
    //printf("General Pair Accuracy : %d / %d (%f%%)\n", counter0 + counter1 , testingPairs.size, accuracyPercentage);
    //printf("%d Identical Pairs accurate\n%d Non Identical pairs accurate\n",counter1,counter0);
    //printf("\n");

    /* --- Clean up ---------------------------------------------------------------------------*/
    
    printf("Cleaning up...\n");

    JobScheduler_Destroy(&jobScheduler);

    Hash_FreeValues(icpToIndex, free);
    Hash_Destroy(icpToIndex);

    Hash_Destroy(indexToIcp);

    List_FreeValues(trainingPairs, Tuple_Free);
    List_Destroy(&trainingPairs);

    List_FreeValues(retrainingPairs, Tuple_Free);
    List_Destroy(&retrainingPairs);

    LogisticRegression_Destroy(model);

    for(int i = 0; i < items.size; i++){
        Hash_FreeValues(xVals[i], free);
        Hash_Destroy(xVals[i]);
    }
    free(xVals);

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

    printf("Exiting...\n\n");

    return 0;
}