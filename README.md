For more information on this project, read the project's [report](./Report.pdf).

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
    -thrd : number of threads
    -bs : batch size
    -train : training steps (1 means no retraining, 2 means 1 retraining step etc.)
    -eq : discard extra pairs so identical and non identical pairs are ALWAYS equal before training


### External libraries :
- acutest.h , used for unit testing, include with command "git submodule update"
