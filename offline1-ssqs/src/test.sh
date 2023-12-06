#!/bin/bash

# compile the main.cpp
g++ main.cpp -o main

# for each folder in IOs folder
for folder in ../IOs/*; do
    ./main  $folder/in.txt
    diff results.txt $folder/results.txt
    rm results.txt
    diff event_orders.txt $folder/event_orders.txt
    rm event_orders.txt

    # if the diff is empty, then the test is passed
    if [ $? -eq 0 ]
    then
        echo "Test passed for $folder"
    else
        echo "Test failed for $folder"
    fi
    
    # new line
    echo ""
done

rm main