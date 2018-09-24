#!/bin/bash

cd output-profile-optimization
times=10

rm time_gauss.csv
echo "-O0,-O1,-O2,-O3,-Ofast" >> time_gauss.csv

counter=1
while [ $counter -le $times ]
do
    echo $counter

    echo -n `./iterativeSolver.O0 G ../data/A_500.csv ../data/b_500.csv 100 2 0 | grep method | awk '{print $7}'` >> time_gauss.csv
    echo -n ',' >> time_gauss.csv
    echo -n `./iterativeSolver.O1 G ../data/A_500.csv ../data/b_500.csv 100 2 0 | grep method | awk '{print $7}'` >> time_gauss.csv
    echo -n ',' >> time_gauss.csv
    echo -n `./iterativeSolver.O2 G ../data/A_500.csv ../data/b_500.csv 100 2 0 | grep method | awk '{print $7}'` >> time_gauss.csv
    echo -n ',' >> time_gauss.csv
    echo -n `./iterativeSolver.O3 G ../data/A_500.csv ../data/b_500.csv 100 2 0 | grep method | awk '{print $7}'` >> time_gauss.csv
    echo -n ',' >> time_gauss.csv
    echo `./iterativeSolver.Ofast G ../data/A_500.csv ../data/b_500.csv 100 2 0 | grep method | awk '{print $7}'` >> time_gauss.csv

    ((counter++))
done

cd ..
