# Κάνοντας compile το <foo>_test.c με μια υλοποίηση <foo>.c του
# συγκεκριμένου τύπου, παράγουμε ένα tets για την υλοποίηση αυτή.

# Υλοποιήσεις μέσω dynamic array: LinkedList
#
LinkedList_Test_OBJS = LinkedList_Test.o $(MODULES)/LinkedList/LinkedList.o
Hash_Test_OBJS = Hash_Test.o $(MODULES)/Hash/Hash.o $(MODULES)/LinkedList/LinkedList.o


# Ο βασικός κορμός του Makefile
include ../common.mk