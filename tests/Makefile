# Κάνοντας compile το <foo>_test.c με μια υλοποίηση <foo>.c του
# συγκεκριμένου τύπου, παράγουμε ένα tets για την υλοποίηση αυτή.

# Υλοποιήσεις μέσω dynamic array: LinkedList
#
LinkedList_Test_OBJS = LinkedList_Test.o $(MODULES)/LinkedList/LinkedList.o
Hash_Test_OBJS = Hash_Test.o $(MODULES)/Hash/Hash.o $(MODULES)/LinkedList/LinkedList.o
ArgUtil_Test_OBJS = ArgUtil_Test.o $(MODULES)/Utils/ArgUtil.o
StringUtil_Test_OBJS = StringUtil_Test.o $(MODULES)/Utils/StringUtil.o $(MODULES)/LinkedList/LinkedList.o $(MODULES)/Hash/Hash.o
JSONParser_Test_OBJS = JSONParser_Test.o $(MODULES)/Utils/JSONParser.o $(MODULES)/LinkedList/LinkedList.o
CliqueGroup_Test_OBJS = CliqueGroup_Test.o $(MODULES)/CliqueGroup/CliqueGroup.o $(MODULES)/Hash/Hash.o $(MODULES)/LinkedList/LinkedList.o $(MODULES)/Hash/Hashes.o $(MODULES)/Utils/JSONParser.o $(MODULES)/Utils/StringUtil.o $(MODULES)/Item/Item.o $(MODULES)/Tuple/Tuple.o
TFIDF_Test_OBJS = TFIDF_Test.o $(MODULES)/Hash/Hashes.o $(MODULES)/Hash/Hash.o $(MODULES)/Utils/StringUtil.o $(MODULES)/TF-IDF/TF-IDF.o $(MODULES)/LinkedList/LinkedList.o $(MODULES)/Hash/Hash.o

# Ο βασικός κορμός του Makefile
include ../common.mk