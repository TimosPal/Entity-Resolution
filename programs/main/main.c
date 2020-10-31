#include <string.h>

#include "Util.h"
#include "ArgUtil.h"
#include "FolderUtil.h"
#include "StringUtil.h"

#include "JSONParser.h"
#include "CSVParser.h"

#include "Hashes.h"
#include "CliqueGroup.h"

#include "Item.h"

/* It is assumed that the json and csv files MUST be correct in format and values
 * so no extra error checking is done on said files , they are parsed right away. */

/* TODO: Read Dataset W and edit hashtable accordingly */

int main(int argc, char* argv[]){
    // Get the flags from argv.
    // -f should contain the path to the folder containing the websites folders.
    char *websitesFolderPath;
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-f", &websitesFolderPath), "arg -f is missing or has no value")

    char *dataSetWPath;
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-w", &dataSetWPath), "arg -w is missing or has no value")

    // -b is the bucketsize
    int bucketSize;
    char *bucketSizeStr;
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-b", &bucketSizeStr), "arg -b is missing or has no value")
    IF_ERROR_MSG(!StringToInt(bucketSizeStr, &bucketSize), "Bucket Size should be a number")


    // Open folder from -f (should contain more folder with names of websites)
    List websiteFolders;
    IF_ERROR_MSG(!GetFolderItems(websitesFolderPath, &websiteFolders), "failed to open/close base folder")

    /* Create CliqueGroup structure (complete structure) */
    CliqueGroup cliqueGroup;
    CliqueGroup_Init(&cliqueGroup, bucketSize, RSHash, StringCmp);

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
            char jsonFilePath[BUFFER_SIZE];
            sprintf(jsonFilePath,"%s/%s",websitePath,(char*)(currItem->value));

            char itemID[BUFFER_SIZE]; /*format:  website//idNumber */
            sprintf(itemID,"%s//%s",(char*)(currWebsiteFolder->value), (char*)(currItem->value));

            RemoveFileExtension(itemID);

            /* Create item and insert into items list */
            Item* item = Item_Create(itemID, GetJsonPairs(jsonFilePath));
            /* TODO: make bucket size dynamic */
            CliqueGroup_Add(&cliqueGroup, itemID, strlen(itemID)+1, item);

            currItem = currItem->next;
        }

        currWebsiteFolder = currWebsiteFolder->next;

        List_FreeValues(currItems,free);
        List_Destroy(&currItems);
    }

    // Update cliqueGroup with dataSetW.
    // We apply the simple logic that for items a,b,c : if a == b and b == c then a == c.
    FILE* dataSetFile = fopen(dataSetWPath, "r");
    IF_ERROR_MSG(dataSetFile == NULL, "-w file not found")

    List values;
    CSV_GetLine(dataSetFile, &values); // get rid of columns
    List_FreeValues(values, free);
    while(CSV_GetLine(dataSetFile, &values)) {
        char* id1 = (char*)values.head->value;
        char* id2 = (char*)values.head->next->value;
        char* similarityString = (char*)values.head->next->next->value;
        //printf("%s : %s : %s\n", id1, id2, similarityString);
        int similarity;
        StringToInt(similarityString,&similarity);

        // If the 2 items are similar we merge the cliques.
        if(similarity == 1) {
            CliqueGroup_Update(&cliqueGroup, id1, (int) strlen(id1) + 1, id2, (int) strlen(id2) + 1);
        }

        List_FreeValues(values, free);
    }

    fclose(dataSetFile);

    /* Deletes items ONLY(within cliques list(which has more lists in it)) */
    CliqueGroup_FreeValues(cliqueGroup, Item_Free);
    CliqueGroup_Destroy(cliqueGroup);

    List_FreeValues(websiteFolders,free);
    List_Destroy(&websiteFolders);

    return 0;
}