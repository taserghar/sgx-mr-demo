#ifndef __BLOCK_OPERATION__
#define __BLOCK_OPERATION__

#include <vector>
#include <string>
#include "utils.h"
using namespace std;


class BlockOperations {
public:
        virtual int readBlock( char* data_out, int block_id) = 0;
        virtual int writeBlock( char* data_in, int block_id) = 0;
};


class InEnclaveBlockFile : public BlockOperations {
public:
     int mBlockSize;

     vector<char>fileBuf;
     InEnclaveBlockFile(string pFileName, IOMode_t pIOMode, int pBlockSize, int pRecordLimit, bool pEncryptionEnabled);
     InEnclaveBlockFile() ;
     int getBlockSize();
     int getRecordLimit();
     // return byte reads
     int readBlock(char* pBlock, int pos);
     int writeBlock(char* pBlock, int pos);
     void read(int pos, char* memblock, int size, int* bytesRead);
     void write(int pos, char* memblock, int size, int* status);

     int closeFile();
     static int getBlockHeaderSize();
};

#endif
