#
# In order to execute this "Makefile" just type "make"
#

OBJS 	= main.o List.o ArgUtil.o
SOURCE	= main.c ./modules/LinkedList/List.c ./modules/ArgUtil/ArgUtil.c
HEADER  = ./includes/List.h ./includes/ArgUtil.h
OUT  	= main
CC	= gcc
FLAGS   = -g -c -Wall
# -g option enables debugging mode 
# -c flag generates object code for separate files

$(OUT): $(OBJS)
	$(CC) -g $(OBJS) -o $@

# create/compile the individual files >>separately<< 
main.o: main.c
	$(CC) $(FLAGS) main.c

List.o: ./modules/LinkedList/List.c
	$(CC) $(FLAGS) ./modules/LinkedList/List.c

ArgUtil.o: ./modules/ArgUtil/ArgUtil.c
	$(CC) $(FLAGS) ./modules/ArgUtil/ArgUtil.c


# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
