Contributors:

1) Palaiologos Timotheos AM: 1115201700112

2) Desipris Georgios AM: 1115201700030

INFORMATION:

How to compile: 
    make (compile)
    make run (compile and run all the tests, arguments are in programs folder Makefile)
    make valgrind (same but with valgrind)
    make clean (delete everything made by the Makefiles)


Argument flags:
    -f filename (the folder which contains the e-commerce sites folders)
    -b number (size of HashTable array)
    -w datasetWPath (path to the datasetW.csv)

DATA STRUCTURES USED:

List: Simple generic double linked List used. We need a double linked list to have O(1) removal of Nodes.

HashTable: Generic HashTable using an array of lists (each list representing the entire bucket). Each list contains KeyValue pairs.

CliqueGroup: Custom Structure containing a HashTable and a List of Lists(which contains all the cliques). Used to Add items in cliques(first step, e.g when reading all the jsons) and updating the cliques(e.g when reading the csv).

The list is needed in order to have a list of all the cliques, so we can traverse the items(and cliques) in order, something that can't be done using a hashtable structure.

This structure uses ItemCliquePairs(both in the hashtable and the list), a struct which has a clique(list) pointer and an item pointer.

Using this structure, we can find an item in O(1) and the clique it belongs to(since it is included in the ItemCliquePair).
So, finding if 2 items belong to the same clique is O(1) and doing anything to every item would be O(N) using the List of cliques(something which is hard with only a hachtable).

Clique Merging is essentially a List Merge but the pointers of the ItemCliquePairs are all updated so it is done in O(N).


UNIT TESTING LIBRARY acutest.h USED.


