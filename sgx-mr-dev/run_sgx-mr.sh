#!/bin/bash

# SGX-MR

##### Constants

APP="./app"

# Default is Bitonic Sort = 1, Merge Sort = 2
# Justin switched to 2 from 1
SORT="2" 
# padding enable: 1 disable: 0
PADDING="0"

BLOCK_SIZE="20480"

#Originally 100
BLOCK_COUNT="25"

# KMeans Parameters
KMEANS_ENCODER="kmeans-encoder"
KMEANS_RUN="kmeans"
KMEANS_CENTROID_FILE="../data/input/centroids.txt"
KMEANS_COORDINATE_FILE="../data/input/coordinates.txt"
KMEANS_OUTPUT_PATH="../data/output/"


# WordCount Parameters
WC_ENCODER="wordcount-encoder"
WC_RUN="wordcount"
WC_PLAIN_TEXT="../data/input/review.json"
WC_BLOCK_FILE="../data/input/block_data.txt"
WC_INPUT_FILE="../data/input/block_data.txt"
WC_OUTPUT_PATH="../data/output/"


##### Main

#$(execute_kmeans_encoder)

if [ "$1" = "kmeans-encoder" ]; then
    command=$APP" "$KMEANS_ENCODER" "$BLOCK_COUNT" "$BLOCK_SIZE" "$KMEANS_CENTROID_FILE" "$KMEANS_COORDINATE_FILE
    echo $command
    $command
elif [ "$1" = "kmeans-run" ]; then
    command=$APP" "$KMEANS_RUN" "$BLOCK_SIZE" "$PADDING" "$SORT" "$KMEANS_CENTROID_FILE" "$KMEANS_COORDINATE_FILE" "$KMEANS_OUTPUT_PATH
    echo $command
    $command
    echo -e "\n\nTimes in miliseconds"
elif [ "$1" = "wc-encoder" ]; then
    command=$APP" "$WC_ENCODER" "$BLOCK_COUNT" "$BLOCK_SIZE" "$WC_PLAIN_TEXT" "$WC_BLOCK_FILE
    echo $command
    $command
elif [ "$1" = "wc-run" ]; then
    command=$APP" "$WC_RUN" "$BLOCK_SIZE" "$PADDING" "$SORT" "$WC_BLOCK_FILE" "$WC_OUTPUT_PATH
    echo $command
    $command
    echo -e "\n\nTimes in miliseconds"
else
    echo "Wrong Argument.Pass one parameter:"
    echo "kmeans-encoder"
    echo "kmeans-run"
    echo "wc-encoder"
    echo "wc-run"
fi


