#ifndef __BLOCKFILE__
#define __BLOCKFILE__

#include "Enclave.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <utility>
#include <string>
#include "BlockOperations.h"
#include "utils.h"
using namespace std;
// #include "untrusted.h"

class BlockFile : public BlockOperations {

public:
    static const int headerSize = 0; //TODO: will increase after adding encryption
    IOMode_t mIOMode;
    int mFileId;
    int mBlockSize;
    int mRecordLimit;
    bool mEncryptionEnabled;

    BlockFile();
    ~BlockFile();
    BlockFile(string pFileName, IOMode_t pIOMode, int pBlockSize, int pRecordLimit, bool pEncryptionEnabled);
    int getBlockSize();
    int getRecordLimit();
    // return byte reads
    int readBlock(char* pBlock, int pos);
    int writeBlock(char* pBlock, int pos);
    int closeFile();
    static int getBlockHeaderSize();
};

#endif
