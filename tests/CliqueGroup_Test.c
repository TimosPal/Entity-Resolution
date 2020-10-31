#include "CliqueGroup.h"
#include "acutest.h"
#include "Hashes.h"
#include "StringUtil.h"
#include "LinkedList.h"
#include "Item.h"
#include "Hash.h"
#include "JsonParser.h"

#include <stdbool.h>

#define INPUT_FILE "../Datasets/camera_specs/2013_camera_specs/buy.net/4233.json"

void CliqueGroup_Test_Add(){
    CliqueGroup cg;
    CliqueGroup_Init(&cg, 10, RSHash, StringCmp);

    char itemID[10] = "test_id";
    Item* item = Item_Create("", GetJsonPairs(INPUT_FILE));
    CliqueGroup_Add(&cg, itemID, strlen(itemID)+1, item);

    /* cliques list is not empty */
    TEST_ASSERT(cg.cliques.size > 0);


    CliqueGroup_FreeValues(cg, Item_Free);
    CliqueGroup_Destroy(cg);
}

TEST_LIST = {
    { "CliqueGroup_test_add",   CliqueGroup_Test_Add },
    { NULL, NULL }
};