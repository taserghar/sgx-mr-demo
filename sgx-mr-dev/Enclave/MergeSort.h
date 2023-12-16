#ifndef __SORT__
#define __SORT__

#include <stdlib.h>
#include <stdio.h>
#include "Writable.h"
#include "RecordStream.h"
#include "BlockFile.h"
#include "utils.h"
#include <algorithm>
using namespace std;

template<class K, class V>
class MergeSort {
public:
    int nob; // no. of blocks
    int blockSize;
    int recordLimit;
    BlockFile* dataMemory;
    BlockFile* additionalmemory;
    MergeSort(){}
    int blockCounter;


    MergeSort(int pNob, int pBlockSize, int pRecordLimit, BlockFile* dataMemory, BlockFile* tempMemory);

    // string getNewFileName();

    void readRecords(vector<Record<K,V>>& records, char* block);

    void writeRecord(BlockFile* storage,
                    RecordOutputStream<K,V>& output,
                    Record<K,V>& rec);

    void flushWriter(BlockFile* storage,
                    RecordOutputStream<K,V>& output);

    void sort();


    void mergeSort(int n);

    void merge(int l, int m, int r);
    bool isDesc(vector<Record<K,V>>& recordList);
    void obliviousBlockMerge(int l, int m, int r);
    void obliviousRecordMerge(vector<Record<K,V>>& records_i,
                        vector<Record<K,V>>& records_j, int dir);
    void obliviousRecordSwap(Record<K,V>&a, Record<K,V>&b, int cond);


    void printRecords(vector<Record<K,V>>& records);
    void printRecord(Record<K,V>&r);
    void printBlock(char* block);

    void printBlockAsRecord(char* block);
};

// void printRecordsFromFile(string inputFileName, int blockSize, int recordLimit);

#endif
