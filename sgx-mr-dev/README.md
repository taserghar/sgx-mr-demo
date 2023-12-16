# sgx-mr-dev

SGX-MR is a prototype of the paper [SGX-MR: Regulating Dataflows for Protecting Access Patterns of Data-Intensive SGX Applications](https://arxiv.org/abs/2009.03518). 

Prototype is for testing the sample applications described in the paper. 
* KMeans
* WordCount


# Prerequisite
SGX-MR is built on top of Intel SGX SDK. To run the executable, it is mandatory to install the recent version of the SGX SDK. Please visit [Linux-SGX](https://github.com/intel/linux-sgx) for setup instruction.

SGX-MR was built, tested, and upload for the below configuration:
* 64-bit Ubuntu 18.04.1 LTS (4.15.0 Kernel)
* gcc version 7.5.0

Please ensure the OS and gcc version before testing.

# Testing
For both Hardware and Simulation mode, a companion shell script (run_sgx-mr.sh) is added to run sample applications with default parameters. It has four options.
* kmeans-encoder
* wc-encoder
* kmeans-run
* wc-run

Here, the First and second parameters are for generating the encrypted block data. For KMeans we generate synthesized data. On the other hand, we used yelp dataset to generate block data for WordCount, as we mentioned in the paper. To download yelp data set, please visit: https://www.kaggle.com/yelp-dataset/yelp-dataset. In this dataset, we set review.json as the default input-text. In case of using other text input for wordcount problem, please change the path in the script. Make sure that input text contains the sufficient data that matches the block size times block count.

Make sure the data directory and its sub-directories are also download if using default parameters.
