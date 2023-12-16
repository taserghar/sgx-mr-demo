#ifndef __BITONIC_SORT__
#define __BITONIC_SORT__

#include <stdlib.h>
#include <stdio.h>
#include "Writable.h"
// #include "BlockFile.h"
#include "RecordStream.h"
#include "utils.h"
#include <algorithm>
#include "BlockOperations.h"
using namespace std;

template<class K, class V>
class BitonicSort {
public:
    BlockOperations* bf;
    int nobExtended;
    int nob; // no. of blocks
    int lastBlockRecordCount;
    int blockSize;
    int recordLimit;
    string outPath;
    string fileName;
    // BitonicSort(){}

    BitonicSort(int pNob, BlockOperations* inputBF, string pPath, int pBlockSize, int pRecordLimit);

    void preprocess();
    string postProcess();
    void bitonicIterative(int nob);

    void compAndSwap(BlockOperations* outFile, BlockOperations* inputFile, int i, int j, int dir);
    string getNewFileName();

    void readRecords(vector<Record<K,V>>& records, char* block);

    int sort(); // returns updated number of block
    bool isDesc(vector<Record<K,V>>& recordList);
    void mergeRecords(vector<Record<K,V>>& records_i,
                        vector<Record<K,V>>& records_j, int dir);
    void simObliviousSwap(Record<K,V>&a, Record<K,V>&b, int cond);
    void obliviousSwap(Record<K,V>&a, Record<K,V>&b, int cond);
    void obliviousMerge(vector<Record<K,V>>& records_i,
                        vector<Record<K,V>>& records_j, int dir);
    void printRecords(vector<Record<K,V>>& records);
    void printRecord(Record<K,V>&r);
    void printBlock(char* block);

    void printBlockAsRecord(char* block);
    void printRecordsFromFile(int nob, int blockSize, int recordLimit);
};



#endif
