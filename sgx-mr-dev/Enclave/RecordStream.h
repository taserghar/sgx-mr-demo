
#ifndef __RECORD_STREAM__
#define __RECORD_STREAM__

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include "utils.h"
#include "BlockFile.h"
#include "Writable.h"

template<class K, class V>
class RecordInputStream {
public:
    char* mBlock;
    int mBlockSize;
    int iter;
    RecordInputStream(char* pBlock);
    bool readRecord(Record<K,V>& pRecord);
    int readRecord(vector<Record<K,V>>& pRecordList);
};


template<class K, class V>
class RecordOutputStream {
public:
    char* mBlock;
    int mBlockSize;
    int mRecordLimit;
    int iter;
    int currentTotalRecord;
    RecordOutputStream();
    ~RecordOutputStream();
    RecordOutputStream(int pBlockSize, int pRecordLmit);
    void setBlock(char* pBlock, int pBlockSize);
    void resetBlock();
    char* getBlock();
    bool isEmpty();
    bool writeRecord(Record<K,V>& pRecord);
    int writeRecord(vector<Record<K,V>>& pRecordList);
};


#endif
