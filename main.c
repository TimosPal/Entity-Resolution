#include "Util.h"
#include "Item.h"


int main(int argc, char* argv[]){

    // Get the flags from argv.
    // -f should contain the path to the folder containing the websites folders.
    char *websitesFolderPath;
    IF_ERROR_MSG(!FindArgAfterFlag(argv, argc, "-f", &websitesFolderPath), "arg -f is missing or has no value")

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
        IF_ERROR_MSG(!GetFolderItems(websitePath, &currItems), "failed to open/close website folder") //NOTE list could be avoided

        /* Create Nodes from the list of Json file names */
        Node* currItem = currItems.head;
        while(currItem != NULL){
            char jsonFilePath[BUFFER_SIZE];
            sprintf(jsonFilePath,"%s/%s",websitePath,(char*)(currItem->value));

            char itemID[BUFFER_SIZE];
            sprintf(itemID,"%s//%s",(char*)(currWebsiteFolder->value), (char*)(currItem->value));
            RemoveFileExtension(itemID);

            Item* item = Item_Create(itemID,GetJsonPairs(jsonFilePath)); // TODO: json pairs isnt working YET.

            currItem = currItem->next;
        }

        currWebsiteFolder = currWebsiteFolder->next;

        List_FreeValues(currItems,free);
        List_Destroy(&currItems);
    }

    List_FreeValues(websiteFolders,free);
    List_Destroy(&websiteFolders);

    return 0;
}