#!/bin/bash

# SGX-MR

##### Constants

# APP="./app"
APP="/demo/sgx-mr-dev/app"

# Default is Bitonic Sort = 1, Merge Sort = 2
# Justin switched to 2 from 1
BITONIC_SORT="1"
MERGE_SORT="2"
# padding enable: 1 disable: 0
PADDING="0"

BLOCK_SIZE="20480"

#Originally 100
BLOCK_COUNT="25"

# KMeans Parameters
KMEANS_ENCODER="kmeans-encoder"
KMEANS_RUN="kmeans"
KMEANS_CENTROID_FILE="/demo/data/input/centroids.txt"
KMEANS_COORDINATE_FILE="/demo/data/input/coordinates.txt"
KMEANS_OUTPUT_PATH="/demo/data/output/"


# WordCount Parameters
WC_ENCODER="wordcount-encoder"
WC_RUN="wordcount"
WC_PLAIN_TEXT="/demo/data/input/bible.txt"
WC_BLOCK_FILE="/demo/data/input/block_data.txt"
WC_INPUT_FILE="/demo/data/input/block_data.txt"
WC_OUTPUT_PATH="/demo/data/output/"



# NOTE: For the demo Encoded data will be precompiled to maintain smaller docker image size.
# The input file we used was originally 5+GB in size. However, we only utilized a small portion of it which was encoded in the docker image.
# If you want to encode new input file. Please encode it using the run_sgx_mr.sh and delete the original input file, if it is large.

##### Main
if [ "$1" = "kmeans-run" ]; then
    # command=$APP" "$KMEANS_ENCODER" "$2" "$BLOCK_SIZE" "$KMEANS_CENTROID_FILE" "$KMEANS_COORDINATE_FILE
    # echo $command
    # $command

    command=$APP" "$KMEANS_RUN" "$BLOCK_SIZE" "$PADDING" "$BITONIC_SORT" "$KMEANS_CENTROID_FILE" "$KMEANS_COORDINATE_FILE" "$KMEANS_OUTPUT_PATH
    echo $command
    $command
    echo -e "\n\nTimes in miliseconds"
    command=$APP" "$KMEANS_RUN" "$BLOCK_SIZE" "$PADDING" "$MERGE_SORT" "$KMEANS_CENTROID_FILE" "$KMEANS_COORDINATE_FILE" "$KMEANS_OUTPUT_PATH
    echo $command
    $command
elif [ "$1" = "wc-run" ]; then
    # command=$APP" "$WC_ENCODER" "$BLOCK_COUNT" "$BLOCK_SIZE" "$WC_PLAIN_TEXT" "$WC_BLOCK_FILE
    # echo $command
    # $command
    command=$APP" "$WC_RUN" "$BLOCK_SIZE" "$PADDING" "$BITONIC_SORT" "$WC_BLOCK_FILE" "$WC_OUTPUT_PATH
    echo $command
    $command
    echo -e "\n\nTimes in miliseconds"
    command=$APP" "$WC_RUN" "$BLOCK_SIZE" "$PADDING" "$MERGE_SORT" "$WC_BLOCK_FILE" "$WC_OUTPUT_PATH
    echo $command
    $command
else
    echo "Wrong Argument.Pass one parameter:"
    echo "kmeans-run"
    echo "wc-run"
fi


