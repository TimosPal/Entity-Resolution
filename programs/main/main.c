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
char** identicalFilePath, char** nonIdenticalFilePath, char** outputFilePath, int* vocabSize, int* epochs, double* maxAccuracyDiff, double* learningRate){
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

void CreateXVals(List items, Hash idfDictionary, Hash itemProcessedWords, double*** XVals, Hash* icpToIndex, Hash* indexToIcp){
    //Array of sparse matrices(hashes) with TFIDF values
    Hash* x = CreateVectors(items, idfDictionary, itemProcessedWords);
    
    //Hash with keys = id of icp and value = index in array of TFIDF vectors
    Hash_Init(icpToIndex, DEFAULT_HASH_SIZE, RSHash, UIntCmp, false);
    Hash_Init(indexToIcp, DEFAULT_HASH_SIZE, RSHash, IcpCmp, false);
    
    //Array with TFIDF vectors for each item
    double **xVals = malloc(items.size * sizeof(double*));
    
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
        xVals[k] = TF_IDF_ToArray(x[k], idfDictionary);

        Hash_FreeValues(x[k], free);
        Hash_Destroy(x[k]);

        k++;
        itemNode = itemNode->next;
    }

    free(x);

    *XVals = xVals;
}

void CreateXY(List items, List pairs, Hash idfDictionary, Hash itemProcessedWords, Hash indexes, unsigned int*** xIndexes, double** Y){
    //Array of vector indexes in TFIDF array for all pairs
    unsigned int** pairIndexes = malloc(pairs.size * sizeof(unsigned int*));
    Node* pairNode = pairs.head;
    int k = 0;
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

void DynamicLearning(CliqueGroup* cliqueGroup, LogisticRegression* model, Hash* idfDictionary, Hash* icpToIndex, Hash* indexToIcp, Hash* itemProcessedWords, List* items, List* trainingPairs, double learningRate, int epochs, List* testingPairs, double** xVals){
    unsigned int width = 2 * idfDictionary->keyValuePairs.size;
    unsigned int height = trainingPairs->size;
    unsigned int** xIndexesTraining;
    double* yValsTraining;
    
    CreateXY(*items, *trainingPairs, *idfDictionary, *itemProcessedWords, *icpToIndex, &xIndexesTraining, &yValsTraining);
    LogisticRegression_Init(model, 0, xVals, xIndexesTraining, yValsTraining, width, height, items->size);
    printf("Created X Y Datasets for training\n\n");
    
    //Testing Datasets
    unsigned int** xIndexesTesting;
    double* yValsTesting;
    
    CreateXY(*items, *testingPairs, *idfDictionary, *itemProcessedWords, *icpToIndex, &xIndexesTesting, &yValsTesting);
    printf("Created X Y Datasets for testing\n\n");
    
    double threshold = THRESHOLD;
    double stepValue = STEP_VALUE;
    int retrainCounter = 0;

    while(threshold < 0.5){
        printf("Starting training #%d...\n", retrainCounter);
        LogisticRegression_Train(model, learningRate, epochs);
        printf("\rTraining completed with %d epochs\n\n", epochs);

        //Start testing
        printf("Predicting #%d...\n", retrainCounter);

        List acceptedPairs;
        List_Init(&acceptedPairs);

        for (int i = 0; i < testingPairs->size; i++) {
            //check if this pair has already beed added to training set before
            if(xIndexesTesting[i][0] == -1){
                continue;
            }

            double* leftVector = xVals[xIndexesTesting[i][0]];
            double* rightVector = xVals[xIndexesTesting[i][1]];
            
            double prediction = LogisticRegression_Predict(model, leftVector, rightVector);
            
            //chance is prediction
            double predictionError = prediction;
            //chance = 1 - error if it is closer to 1
            if (1 - predictionError < predictionError){
                predictionError = 1 - predictionError;
            }
            //if error is under threshold
            if(predictionError < threshold) {
                //Add pair and predictionError tuple to list
                Tuple* PairPredictionErrorTuple = malloc(sizeof(Tuple));
                //we pass the prediction and not the prediction error in order to know how to retrain
                Tuple_Init(PairPredictionErrorTuple, &i, sizeof(int), &prediction, sizeof(double));
                List_Append(&acceptedPairs, PairPredictionErrorTuple);
            }
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

            ItemCliquePair* icp1 = Hash_GetValue(*indexToIcp, &index1, sizeof(unsigned int));
            ItemCliquePair* icp2 = Hash_GetValue(*indexToIcp, &index2, sizeof(unsigned int));
            bool isEqual = (1 - prediction < 0.5) ? true : false;

            //if pair is valid into cliqueGroup
            Item* item1 = icp1->item;
            Item* item2 = icp2->item;
            if (!CliqueGroup_PairIsValid(icp1, icp2, isEqual)){
                printf("1\n");
                isEqual = !isEqual;
            }

            if (!CliqueGroup_PairIsValid(icp1, icp2, isEqual)){
                printf("2\n");
            }

            if(isEqual){
                CliqueGroup_Update_Similar(cliqueGroup, item1->id, strlen(item1->id)+1, item2->id, strlen(item2->id)+1);
            }else{
                CliqueGroup_Update_NonSimilar(cliqueGroup, item1->id, strlen(item1->id)+1, item2->id, strlen(item2->id)+1);
            }
        }
        
        //Finalize cliqueGroup
        CliqueGroup_Finalize(*cliqueGroup);
        
        //Get Pairs that will be trained in the next loop


        //Cleanup
        List_FreeValues(acceptedPairs, Tuple_Free);
        List_Destroy(&acceptedPairs);
        free(acceptedPairsArray);

        //increment threshold and retrainCounter
        retrainCounter++;
        threshold += stepValue;
    }

    //Cleanup
    free(yValsTesting);
    for(int i = 0; i < testingPairs->size; i++){
        free(xIndexesTesting[i]);
    }
    free(xIndexesTesting);
}

int main(int argc, char* argv[]){
    /* --- Arguments --------------------------------------------------------------------------*/

    char *websitesFolderPath , *dataSetWPath, *identicalFilePath, *nonIdenticalFilePath, *outputFilePath;
    int bucketSize, vocabSize, epochs;
    double maxAccuracyDiff, learningRate;
    ParseArgs(argc, argv, &websitesFolderPath, &dataSetWPath, &bucketSize, &identicalFilePath, &nonIdenticalFilePath, &outputFilePath, &vocabSize, &epochs, &maxAccuracyDiff, &learningRate);
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
    printf("%d identical pairs found, printed in %s\n", trainingPairs.size, identicalFilePath);
    
    //Redirect stdout to the fd of the identicalFilePath
    int fd_copy, fd_new;
    RedirectFileDescriptorToFile(1, identicalFilePath, &fd_new, &fd_copy);
    CliqueGroup_PrintPairs(trainingPairs, Item_Print);
    ResetFileDescriptor(1, fd_new, fd_copy);

    //Get Non Identical Pairs
    List nonIdenticalPairs = CliqueGroup_GetNonIdenticalPairs(&cliqueGroup);
    printf("%d non identical pairs found, printed in %s\n\n", nonIdenticalPairs.size, nonIdenticalFilePath);
    
    //Redirect stdout to the fd of the nonIdenticalFilePath
    RedirectFileDescriptorToFile(1, nonIdenticalFilePath, &fd_new, &fd_copy);
    CliqueGroup_PrintPairs(nonIdenticalPairs, Item_Print);
    ResetFileDescriptor(1, fd_new, fd_copy);

    // Join lists for later training.
    List_Join(&trainingPairs, &nonIdenticalPairs);

    //Shuffle list of training pairs before splitting
    List_Shuffle(&trainingPairs);

    // Split the set 60-40
    double trainingPercentage = 0.6, testingPercentage = 0.2, validationPercentage = 0.2;

    List testingPairs;
    IF_ERROR_MSG(!List_Split(&trainingPairs, &testingPairs, trainingPercentage), "Can't split dataset for testing")
    printf("Split for testing\n");

    List validationPairs;
    IF_ERROR_MSG(!List_Split(&testingPairs, &validationPairs, testingPercentage + trainingPercentage/2), "Can't split dataset for validation")
    printf("Split for validation\n\n");

    printf("Training size: %d pairs (%.2f%%)\nTesting size: %d pairs (%.2f%%)\nValidation size: %d pairs (%.2f%%)\n\n",
    trainingPairs.size, trainingPercentage*100, validationPairs.size, testingPercentage*100, validationPairs.size, validationPercentage*100);

    //Get all items in a list to use later
    List items = CliqueGroup_GetAllItems(cliqueGroup);
    
    /* --- Create processed words for items ---------------------------------------------------*/

    Hash itemProcessedWords = CreateProcessedItems(cliqueGroup);
    printf("Created Processed Words\n");
    Hash idfDictionary = IDF_Calculate(items, itemProcessedWords, vocabSize); //Create Dictionary based on items list
    printf("Created and Trimmed Dictionary based on average TFIDF\n");


    double** xVals;
    Hash icpToIndex, indexToIcp;
    CreateXVals(items, idfDictionary, itemProcessedWords, &xVals, &icpToIndex, &indexToIcp);
    LogisticRegression model;

    //Training
    
    DynamicLearning(&cliqueGroup, &model, &idfDictionary, &icpToIndex, &indexToIcp, &itemProcessedWords, &items, &trainingPairs, learningRate, epochs, &testingPairs, xVals);    

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

    Hash_FreeValues(icpToIndex, free);
    Hash_Destroy(icpToIndex);

    Hash_Destroy(indexToIcp);

    List_FreeValues(trainingPairs, Tuple_Free);
    List_Destroy(&trainingPairs);

    LogisticRegression_Destroy(model);

    for(int i = 0; i < items.size; i++){
        free(xVals[i]);
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