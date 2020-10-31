#include "CliqueGroup.h"
#include "acutest.h"
#include "Hashes.h"
#include "StringUtil.h"
#include "LinkedList.h"
#include "Item.h"
#include "Hash.h"
#include "JSONParser.h"

#include <stdbool.h>

#define INPUT_FILE "../Datasets/camera_specs/2013_camera_specs/buy.net/4233.json"

void CliqueGroup_Test_Add(){
    CliqueGroup cg;
    CliqueGroup_Init(&cg, 10, RSHash, StringCmp);

    char item1ID[10] = "test_id1";
    Item* item1 = Item_Create(item1ID, GetJsonPairs(INPUT_FILE));
    CliqueGroup_Add(&cg, item1ID, strlen(item1ID)+1, item1);

    char item2ID[10] = "test_id2";
    Item* item2 = Item_Create(item2ID, GetJsonPairs(INPUT_FILE));
    CliqueGroup_Add(&cg, item2ID, strlen(item2ID)+1, item2);

    /* cliques list is not empty */
    TEST_ASSERT(cg.cliques.size > 0);


    CliqueGroup_FreeValues(cg, Item_Free);
    CliqueGroup_Destroy(cg);
}

TEST_LIST = {
    { "CliqueGroup_test_add",   CliqueGroup_Test_Add },
    { NULL, NULL }
};