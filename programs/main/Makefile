# Λίστα με objects (.o) για κάθε εκτελέσιμο που θέλουμε να φτιάξουμε.
#

main_OBJS		= main.o $(LIB)/ER.a

# Ορίσματα που χρησιμοποιούνται από το make run
# ../../Datasets/sigmod_large_labelled_dataset.csv
main_ARGS = -f ../../Datasets/camera_specs/2013_camera_specs -w ../../Datasets/sigmod_medium_labelled_dataset.csv -sw ../../stopwords.txt -o ../.. -v 10 -e 2 -d 0.5 -r 2 -thrd 10 -bs 2 -eq 0 -train 2
#main_ARGS = -f ../../Datasets/camera_specs/test_specs -w ../../Datasets/test.csv

# Ο βασικός κορμός του Makefile
include ../../common.mk

# Για να φτιάξουμε το ER.a, τρέχουμε το make στο lib directory.
# Αυτό γίνεται ακόμα και αν υπάρχει ήδη το ER.a (.PHONY), γιατί ίσως πρέπει να ενημερωθεί.
.PHONY: $(LIB)/ER.a
$(LIB)/ER.a:
	$(MAKE) -C $(LIB) ER.a
