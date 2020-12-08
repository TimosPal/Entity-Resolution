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

void ParseArgs(int argc, char* argv[], char **websitesFolderPath,char **dataSetWPath,int* bucketSize){
    // Get the flags from argv.
    // -f should contain the path to the folder containing the websites folders.
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-f", websitesFolderPath), "arg -f is missing or has no value")
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-w", dataSetWPath), "arg -w is missing or has no value")

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
    Hash_Init(&stopwords, DEFAULT_HASH_SIZE, RSHash, StringCmp);

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

Hash CreateProcessedItems(CliqueGroup cg){
    Hash itemProcessedWords;
    Hash_Init(&itemProcessedWords, cg.hash.bucketSize, cg.hash.hashFunction, cg.hash.cmpFunction);

    Hash stopwords = CreateStopwordHash(STOPWORDS_FILE);

    Node* currCliqueNode = cg.cliques.head;
    while (currCliqueNode != NULL){
        Clique* currClique = (Clique*)(currCliqueNode->value);
        List currCliqueList = currClique->similar;

        Node* currIcpNode = currCliqueList.head;
        while(currIcpNode != NULL){
            ItemCliquePair* icp = (ItemCliquePair*)(currIcpNode->value);
            Item* item = icp->item;

            List* itemWords = Item_Preprocess(item, stopwords);

            Hash_Add(&itemProcessedWords, item->id, strlen(item->id)+1, itemWords);

            currIcpNode = currIcpNode->next;
        }

        currCliqueNode = currCliqueNode->next;
    }

    Hash_Destroy(stopwords);

    return itemProcessedWords;
}

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
            int keySize = strlen(currWord) + 1;

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
    
    return dictionary;
}

int main(int argc, char* argv[]){
    /* --- Arguments --------------------------------------------------------------------------*/

    char *websitesFolderPath , *dataSetWPath;
    int bucketSize;
    ParseArgs(argc, argv, &websitesFolderPath, &dataSetWPath, &bucketSize);

    /* --- Reads Json files and adds them to the clique ---------------------------------------*/

    CliqueGroup cliqueGroup;
    CliqueGroup_Init(&cliqueGroup, bucketSize, RSHash, StringCmp);
    HandleData_X(websitesFolderPath,bucketSize,&cliqueGroup);

    /* --- Reads CSV files and updates the cliqueGroups ---------------------------------------*/

    HandleData_W(dataSetWPath,&cliqueGroup);

    /* --- Print results ----------------------------------------------------------------------*/

    //CliqueGroup_PrintIdentical(&cliqueGroup, Item_Print);

    /* --- Create processed words for items ---------------------------------------------------*/

    Hash itemProcessedWords = CreateProcessedItems(cliqueGroup);
    Hash dictionary = CreateIDF(*(Clique*)cliqueGroup.cliques.tail->value,itemProcessedWords,0);

    /* --- Clean up ---------------------------------------------------------------------------*/

    Hash_FreeValues(dictionary, free);
    Hash_Destroy(dictionary);

    Hash_FreeValues(itemProcessedWords, WordList_Free);
    Hash_Destroy(itemProcessedWords);
    
    CliqueGroup_FreeValues(cliqueGroup, Item_Free);
    CliqueGroup_Destroy(cliqueGroup);

    return 0;
}