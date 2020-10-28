#
# In order to execute this "Makefile" just type "make"
#

MOD_DIR = ./modules
OBJS 	= main.o List.o ArgUtil.o FolderUtil.o StringUtil.o
SOURCE	= main.c $(MOD_DIR)/LinkedList/List.c $(MOD_DIR)/ArgUtil/ArgUtil.c $(MOD_DIR)/Utils/FolderUtil.c $(MOD_DIR)/Utils/StringUtil.c $(MOD_DIR)/Hash/Hash.c
HEADER  = List.h ArgUtil.h
OUT  	= main
CC	= gcc
INC_DIR = ./includes
FLAGS   = -g -c -Wall -I$(INC_DIR)

# -g option enables debugging mode 
# -c flag generates object code for separate files

$(OUT): $(OBJS)
	$(CC) -g $(OBJS) -o $@

# create/compile the individual files >>separately<< 
main.o: main.c
	$(CC) $(FLAGS) main.c

List.o: $(MOD_DIR)/LinkedList/List.c  
	$(CC) $(FLAGS) $(MOD_DIR)/LinkedList/List.c

ArgUtil.o: $(MOD_DIR)/Utils/ArgUtil.c 
	$(CC) $(FLAGS) $(MOD_DIR)/Utils/ArgUtil.c

FolderUtil.o: $(MOD_DIR)/Utils/FolderUtil.c
	$(CC) $(FLAGS) $(MOD_DIR)/Utils/FolderUtil.c

StringUtil.o: $(MOD_DIR)/Utils/StringUtil.c
	$(CC) $(FLAGS) $(MOD_DIR)/Utils/StringUtil.c

Hash.o: $(MOD_DIR)/Hash/Hash.c
	$(CC) $(FLAGS) $(MOD_DIR)/Hash/Hash.c


# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
