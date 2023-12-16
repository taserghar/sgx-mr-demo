/*
 * Copyright (C) 2011-2019 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include "sgx_tcrypto.h"
#include "sgx_trts.h"
#include <stdio.h> /* vsnprintf */
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>


#include "sgx_tcrypto.h"
#include "sgx_trts.h"
#include "synthetic_data.h"
#include "utils.h"
#include "BlockFile.h"
#include "Seq.h"
#include "BitonicSort.h"
#include "crypto.h"
#include "wordcount.h"
#include "join_merge.h"
#include "kmeans.h"
#include "oasm_lib.h"

//Justin
#include "DemoLog.h"

using namespace std;

const char* TAG = "[ENCLAVE]";
const char* NL = "\n";
const char* SPACE = " ";
const char* DELIM = " ,.*()[]{}?!\"\t\r\n\f";
const int ONE = 1;
const char* ONE_STR = "1";


/** MAP **/
map<string, vector<string>> seqMap;

string getSyntheticFileName(int nob) {
    return "synthetic_bsize_" + to_string(nob) + ".txt";
}
//sort parameters
BlockFile bf;
InEnclaveBlockFile ibf;
// BitonicSort<StringWritable,StringWritable> bs;
int blockSize;
int recordLimit;
int recordSize;
int NOB;
string outPath = "../data/test";
void prepareSort(int p_nob, int p_blockSize, int p_recordSize, int p_recordLimit, int isInEnclaveMemory) {
    // string fileName = "data/output/map.out";
    string fileName = "../data/test/sort_data.txt";
    blockSize = p_blockSize;
    recordSize = p_recordSize;
    recordLimit = p_recordLimit;
    NOB = p_nob;
    if (isInEnclaveMemory) {
        LOG("using InEnclave Memory\n");
        ibf = InEnclaveBlockFile(fileName, IOMode_READ, blockSize, recordLimit, false);
    } else {
        LOG("using Buffered Memory\n");
        bf = BlockFile(fileName, IOMode_READ, blockSize, recordLimit, false);
    }
}

void runSort(int isInEnclaveMemory) {
    LOG("%s\n", isInEnclaveMemory? "Enclave Memory" : "Buffer Memory");
    if (isInEnclaveMemory) {
        BitonicSort<StringWritable,StringWritable> bs(NOB, (BlockOperations*)&ibf, outPath, blockSize, recordLimit);
        // bs.sort();
        bs.printRecordsFromFile(NOB, blockSize, recordLimit);
    } else {
        BitonicSort<StringWritable,StringWritable> bs(NOB, (BlockOperations*)&bf, outPath, blockSize, recordLimit);
        // bs.sort();
        bs.printRecordsFromFile(NOB, blockSize, recordLimit);
    }


}

void postSort(int isInEnclaveMemory) {
    if (!isInEnclaveMemory) bf.closeFile();
}

void ecall_wordcount(int blockSize, int pEnablePadding, int pSortType, const char* p_inputFile, const char* p_outPath){
    string inputFile(p_inputFile);
    string outPath(p_outPath);
    wordcount(blockSize, pEnablePadding, pSortType, inputFile, outPath);
}

void ecall_encoder_join(int nob) {
    encoder_join(nob);
}

void ecall_merge_join(int p_enablePadding, int p_sort_type){
    join_merge(p_enablePadding, p_sort_type);
}

void ecall_kmeans(int blockSize, int pEnablePadding, int pSortType, const char* pCentroidFile, const char* pCoordinateFile, const char* pOutputPath) {
    string centroidFile(pCentroidFile);
    string coordinateFile(pCoordinateFile);
    string outputPath(pOutputPath);
    kmeans( blockSize, pEnablePadding, pSortType, centroidFile, coordinateFile, outputPath);
}

void ecall_kmeans_encoder(int pTotalBlock, int pBlockSize, const char* pCentroidFile, const char* pCoordinateFile) {
    string centroidFile(pCentroidFile);
    string coordinateFile(pCoordinateFile);
    kmeans_encoder(pTotalBlock, pBlockSize, centroidFile, coordinateFile);
}

void ecall_encoder(int totalBlock, int blockSize, const char* pInputFile, const char* pOutputFile ) {
    LOG(" %d %d %s\n", totalBlock, blockSize, pInputFile);
    // return;
    // input file parameters
    string inputFile(pInputFile); //"../data/plain_data/review.json";
    int xTimes = (blockSize + 2048 -1) / 2048;
    int inputBlockSize = /*blockSize;//*/1024 * xTimes; // changing for test
    int outputBlockSize = blockSize;//2048 * xTimes;
    recordLimit = 30 * xTimes;
    char* pBlock = new char[inputBlockSize];
    int iter = 0;
    BlockFile bf = BlockFile(inputFile, IOMode_READ, inputBlockSize, recordLimit, false);
    // output file parameters

    string outputFile(pOutputFile);  //"../data/input/block_data.txt";
    int blockCnt = 0;
    //TODO: TODO: TODO: Must make true after testing
    BlockFile outputBF = BlockFile(outputFile, IOMode_WRITE, outputBlockSize, recordLimit, true);


    while (bf.readBlock(pBlock, iter++) != 0){
        // LOG("Read Line: %d %s\n", iter, pBlock);
            vector<Record<StringWritable,StringWritable>> records;
            // NOTE: arbitary size of string will not pass through the ecnryption
            // TODO: issue is on converting pBlock to string need to fix this
            string inputString;
            for( int i =0 ; i < inputBlockSize; i++) {
                // LOG("Pushing:%d) %c\n", i, pBlock[i]);
                inputString.push_back(pBlock[i]);
                // LOG("Pushing Done \n");
            }
            // LOG("LINE: %s", inputString.c_str());
            StringWritable K(to_string(iter));
            StringWritable V(inputString);
            Record<StringWritable,StringWritable>rec(K,V);
            records.push_back(rec);
            // LOG("Converting to record stream\n");
            RecordOutputStream<StringWritable, StringWritable>outStream(outputBlockSize, recordLimit);
            outStream.writeRecord(records);
            char* block = outStream.getBlock();

             // LOG("Calling writeBlock %d\n", iter);
           outputBF.writeBlock(block, blockCnt++);
            // delete[] block;
            records.clear();
            free(block);
            if( iter == totalBlock) break;

    }
    LOG("Total Block: %d\n", iter);
    outputBF.closeFile();
    bf.closeFile();
}

void gen_data(int p_nob, int p_blockSize, int p_recordSize, int p_recordLimit) {
    gen_synthetic( p_nob,  p_blockSize,  p_recordSize,  p_recordLimit);
    printf("------------------------- Data Generation Done -------------\n");
}
char* memtestArray;

void ecall_prepareMemtest(int n) {
    memtestArray = (char*) malloc(n);

    if( memtestArray ) {
        LOG("Allocation Successful\n");
        for( int i =0 ; i < n; i++) memtestArray[i] =  getRandomNumber();
    } else {
        LOG("Allocation failed\n");
    }
}

void ecall_memtest( int n, int isSeq) {
    LOG("%d %d\n", n, isSeq);
    if(!isSeq) {
        for( int i =0 ; i < n; i++) {
            size_t ind =  getRandomNumber() % n;
            int p = memtestArray[ind];
            int q = p >> 1;
            swap(p,q);
        }
    } else {
        for( int i =0; i < n; i++) {
            int p = memtestArray[i];
            int q = p >> 1;
            swap(p,q);
        }
    }
}

void ecall_oswap_test();

string str_a;// = "AAAAAAAA";
string str_b;// = "BBBBBBBB";
string str_c;
size_t str_siz = 20480;
void ecall_prepare_test(){
    for( int i =0; i < str_siz; i++) {
        str_a.push_back('A');
        str_b.push_back('B');
        str_c.push_back('C');
    }
}
void ecall_test(){
    ecall_oswap_test();
}

void ecall_oswap_test() {
    for( int i = 0; i < 1000; i++) {
        oswap_buffer((unsigned char*)&str_a[0], (unsigned char*)&str_b[0], str_siz, 1);
        // string temp = str_a;
        // str_a = str_b;
        // str_b = temp;
        // swap(str_a,str_b);
        // swap(str_a,str_c);
        // swap(str_b,str_c);
    }
    // LOG("oswap_buffer:\n[A]%s\n[B]%s\n",str_a.c_str(), str_b.c_str());

}

void testEncryption() {
    int data_size = 512;
    uint8_t* plain_data = (uint8_t*) malloc(data_size);
    uint8_t* enc_data = (uint8_t*) malloc(data_size);
    uint8_t* dec_data = (uint8_t*) malloc(data_size);
    for( int i = 0; i < data_size; i++) {
        plain_data[i] ='Z';
    }
    aes_encrypt(plain_data, enc_data, data_size);
    memset(plain_data, 0, data_size);
    aes_decrypt(enc_data, dec_data, data_size);

    printf("Encrypted Data\n");
    for( int i = 0; i < data_size; i++) {
        printf("%c", enc_data[i]);
    }
    printf("Reverted Data\n");
    for( int i = 0; i < data_size; i++) {
        printf("%c", dec_data[i]);
    }
    printf("\n");
}

/*
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf(const char* fmt, ...)
{
    char buf[BUFSIZ] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;
}
