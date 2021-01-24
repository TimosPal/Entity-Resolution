#!/bin/bash
make clean;
make;

learning_rates=( 2 0.2 0.02 );
epochs1=( 6 12 24);
datasets=( Datasets/sigmod_medium_labelled_dataset.csv Datasets/sigmod_large_labelled_dataset.csv );
batch_sizes=( 256 512 1024 );

#WITHOUT RETRAINING
for dataset in "${datasets[@]}"
do
    for learning_rate in "${learning_rates[@]}"
    do
        for epoch in "${epochs1[@]}"
        do
            ./programs/main/main -f Datasets/camera_specs/2013_camera_specs -w $dataset -sw stopwords.txt -o . -v 1000 -e $epoch -d 0.5 -r $learning_rate -thrd 10 -bs ${batch_sizes[0]} -eq 0 -train 1;
        done
    done
done

#BATCH SIZE
for dataset in "${datasets[@]}"
do
    for batch_size in "${batch_sizes[@]}"
    do
        ./programs/main/main -f Datasets/camera_specs/2013_camera_specs -w $dataset -sw stopwords.txt -o . -v 1000 -e ${epochs1[2]} -d 0.5 -r ${learning_rates[0]} -thrd 10 -bs $batch_size -eq 0 -train 1;
    done
done

#EQUAL PAIRS MEDIUM
equal_pairs=( 0 1 );
epochs2=( 10 100 );
learning_rates2=( 20 2 0.2);

for learning_rate in "${learning_rates2[@]}"
do
    for epoch in "${epochs2[@]}"
    do
        ./programs/main/main -f Datasets/camera_specs/2013_camera_specs -w ${datasets[1]} -sw stopwords.txt -o . -v 1000 -e $epoch -d 0.5 -r $learning_rate -thrd 10 -bs ${batch_sizes[1]} -eq 1 -train 1;
    done
done


#EQUAL PAIRS LARGE
epochs3=( 10 100);

for learning_rate in "${learning_rates2[@]}"
do
    for epoch in "${epochs3[@]}"
    do
        ./programs/main/main -f Datasets/camera_specs/2013_camera_specs -w ${datasets[1]} -sw stopwords.txt -o . -v 1000 -e $epoch -d 0.5 -r $learning_rate -thrd 10 -bs ${batch_sizes[1]} -eq 1 -train 1;
    done
done


#THREAD COUNT
thread_counts=( 1 2 4 8 );
for thread_count in "${thread_counts[@]}"
do
    ./programs/main/main -f Datasets/camera_specs/2013_camera_specs -w ${datasets[1]} -sw stopwords.txt -o . -v 1000 -e 10 -d 0.5 -r ${learning_rates[0]} -thrd $thread_count -bs ${batch_sizes[1]} -eq 0 -train 1;
done


#WITH RETRAINING
training_steps=( 1 2 4 );
for training_step_val in "${training_steps[@]}"
do
    ./programs/main/main -f Datasets/camera_specs/2013_camera_specs -w ${datasets[1]} -sw stopwords.txt -o . -v 1000 -e 10 -d 0.5 -r ${learning_rates[0]} -thrd 10 -bs ${batch_sizes[1]} -eq 1 -train $training_step_val;
done

