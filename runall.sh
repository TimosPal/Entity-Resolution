#!/bin/bash
learning_rates=( 1 0.1 0.01 0.001 0.0001 );
epochs=( 1 2 4 8 16 );
datasets=( Datasets/sigmod_medium_labelled_dataset.csv Datasets/sigmod_large_labelled_dataset.csv );
thread_counts=( 1 2 4 8 16);
batch_sizes=( 1 512 1024 );
training_steps=( 2 4 );
equal_pairs=( 0 1 );

make clean;
make;

#WITHOUT RETRAINING
for dataset in $datasets; do
    for learning_rate in $learning_rates; do
        for epoch in $epochs; do
            for batch_sizes_val in $batch_sizes; do
                for equal_pairs_val in $equal_pairs; do
                    ./programs/main/main -f Datasets/camera_specs/2013_camera_specs -w $dataset -sw stopwords.txt -o . -v 1000 -e $epoch -d 0.5 -r $learning_rate -thrd 1 -bs $batch_sizes_val -eq $equal_pairs_val -train 1
                done
            done
        done
    done
done



#WITH RETRAINING


