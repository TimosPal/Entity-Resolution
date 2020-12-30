### Concept : 
We read various json files , we add them to a custom data structure called
CliqueGroup , by reading a csv file we update said structure by merging cliques
of similar items together.
Eg : if a == b and b == c then a == c.

We also create "negative" connections between non similar items.
Eg : if a == b and b != c then a != c

We then feed all the produced pairs into a logistic regression model that uses
stochastic gradient descent for training.
The data is split into 3 groups (train 60% , test 20% , validation 20%)
Our model predicts whether or not two items are similar. This is
achieved by calculating the TF-IDF values of all the pairs and concatenating them
together. 

Since the word set is very big we reduce the dimensionality of our vectors by
trimming the lowest frequencies based on the average tf-idf values. Prepossessing is also
done to the json files by removing unicodes - non printable asci characters , stopwords , etc.

To reduce memory usage we dont calculate each possible concatenated vector but only one per unique
item. A index array is produced so when a pair is read we know which two tf-idf sub-vectors to look for

So instead of allocating X_pairs * 2 * dictionary_length * sizeof(double) bytes we create
X_items * dictionary_length * sizeof(double) + X_pairs * 2 * sizeof(int) bytes. Dropping the memory ussage from
5gb to 1gb. (eg if we got pairs 1:2 , 1:3 , 1:4 , tfidf values for item 1 are only stored once)

Wherever possible sparse matrices are also used but only before the training step.

The predictions of the test set , the final accuracy and all the produced pairs are
stored in appropriate files


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
    -b : size of HashTable array used in cliqueGroup (optional)
    -w : path to the datasetW csv file
    -i : where the produced simillar pairs should be saved
    -n : where the produced non-simillar (non-identical) pairs should be saved
    -o : where the predictions and accuracy of the testing test should be saved
    -v : vocabulary size for the dictionary
    -e : number of epochs for training the model
    -d : max accuracy difference, > 0 and <= 1> (if below this percentage, the difference is acceptable in testing and the result was accurate).
    -r : learning rate for model training (double)

### Data structured used :

-   Tuple: basic generic tuple of two values.

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
    
    Clique_Finalize has to be called after all the items are added so possible duplicates are removed
    from the Clique non-smimilar lists. Inside the clique structure a hash is updated with the appropriate
    non-similar cliques keys for easier O(1) access. 

### External libraries :
- acutest.h , used for unit testing

### Contributors :
- Palaiologos Timotheos AM: 1115201700112
- Desipris Georgios AM: 1115201700030
