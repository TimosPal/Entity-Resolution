### Concept : 
We read various json files , we add them to a custom data structure called
CliqueGroup , by reading a csv file we update said structure by merging cliques
of similar items together.
Eg : if a == b and b == c then a == c.


### How to compile / Makefile usage :
If run through make , the arguments used are defined in the main's 
Makefile located in programs/main
 
    make (compiles everything , can be run from programs/main afterwards)
    make run (run all the tests and the programs)
    make run-programs (run only the programs)
    make run-tests (run only the tests)
    make valgrind (run all the tests and the programs with valgrind)
    make valgrind-tests (run only the tests with valgrind)
    make valgrind-programs (run only the programs with valgrind)
    make clean (delete everything made by the Makefiles)

### Argument flags :
    -f : the folder which contains the folders with the jsons files.
    -b : size of HashTable array used in cliqueGroup
    -w : path to the datasetW csv file

### Data structured used :

-   List: Generic double linked List. We need a double linked list to have O(1)
    removal of Nodes inside cliqueGroup.

-   HashTable: Generic HashTable using an array of lists (each list representing
    the entire bucket). Each list contains KeyValue pairs.

-   CliqueGroup: Custom Structure containing a HashTable and a List of 
    Lists (which contains all the cliques). Used to Add items in cliques
    (e.g when reading all the jsons) and updating the cliques (e.g when reading the csv file).

    The internal list is needed in order to have a list of all the cliques, so we can traverse the 
    items (and cliques) in order.
    
    This structure uses ItemCliquePairs (both in the hashtable and the list), a struct which has 
    a clique(list) pointer and an item pointer.
    
    We can find an item in O(1) complexity through the hashtable and the clique it belongs to (since 
    it is included in the ItemCliquePair). Finding if two items belong to the same clique is 
    also O(1) by comparing the clique addresses.
    
    Clique Merging is essentially a List Merge but the pointers of the ItemCliquePairs are all 
    updated so it is done in O(N).

### External libraries :
- acutest.h , used for unit testing

### Contributors :
- Palaiologos Timotheos AM: 1115201700112
- Desipris Georgios AM: 1115201700030
