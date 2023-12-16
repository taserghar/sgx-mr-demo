#include "BlockFile.h"
#include "utils.h"
#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include "crypto.h"
using namespace std;

BlockFile::BlockFile(){}
BlockFile::BlockFile(string pFileName, IOMode_t pIOMode, int pBlockSize, int pRecordLimit, bool pEncryptionEnabled) {
    static int fileCounter = 0;
    mFileId = ++fileCounter;
    mIOMode = pIOMode;
    mBlockSize = pBlockSize;
    mRecordLimit = pRecordLimit;
    mEncryptionEnabled = pEncryptionEnabled;
    char* fname = (char*) malloc( pFileName.length());
    memset(fname, 0, pFileName.length());
    memcpy(fname,pFileName.c_str(), pFileName.length() );
    // printf("%s  :::: %s\n", __func__, fname);
    ocall_OpenFile(pFileName.c_str(), mFileId, (int) pIOMode);
}

BlockFile::~BlockFile() {
    // LOG("\n");
    if (mFileId) {
       // ocall_CloseFile(mFileId);
    }
}

int BlockFile::getBlockSize() {
    return mBlockSize;
}

int BlockFile::getRecordLimit() {
    return mRecordLimit;
}

int BlockFile::readBlock(char* pBlock, int pos) {

    int byteReads = 0;
    if (mEncryptionEnabled) {
	//LOG("encrypted in readBlock: pos = %d, mBlockSize = %d\n", pos, mBlockSize);
        int bytePosition = pos * (mBlockSize + CMAC_SIZE);

        uint8_t* input_block = (uint8_t*) malloc(mBlockSize + CMAC_SIZE);
        uint8_t* protected_block = (uint8_t*) malloc(mBlockSize + CMAC_SIZE); // with MAC
        uint8_t* cmac_tag = (uint8_t*) malloc(CMAC_SIZE);

        ocall_ReadFile(mFileId, bytePosition, (char*)input_block, mBlockSize + CMAC_SIZE, &byteReads);
        aes_decrypt( input_block, (uint8_t*) protected_block, mBlockSize + CMAC_SIZE);
        //integrity checking
        memcpy(pBlock, protected_block + CMAC_SIZE, mBlockSize);
        compute_CMAC((uint8_t*)pBlock, mBlockSize, cmac_tag);
        int integrity_status = memcmp ( cmac_tag, protected_block, CMAC_SIZE );
        if(integrity_status!=0) {
            // LOG("Integrity Issue: Block # %d Tempered\n", pos);
            // LOG("Computed MAC #### %0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x\n", cmac_tag[0],cmac_tag[1],cmac_tag[2],cmac_tag[3],cmac_tag[4],
            // cmac_tag[5],cmac_tag[6], cmac_tag[7],cmac_tag[8],cmac_tag[9],cmac_tag[10],
            // cmac_tag[11],cmac_tag[12],cmac_tag[13],cmac_tag[14],cmac_tag[15]);
            //
            // LOG("Received MAC #### %0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x\n", pBlock[0],pBlock[1],pBlock[2],pBlock[3],pBlock[4],
            // pBlock[5],pBlock[6], pBlock[7],pBlock[8],pBlock[9],pBlock[10],
            // pBlock[11],pBlock[12],pBlock[13],pBlock[14],pBlock[15]);
        }
        free(input_block);
        free(protected_block);
        free(cmac_tag);
    } else {

	//LOG("in readBlock: pos = %d, mBlockSize = %d\n", pos, mBlockSize);
        int bytePosition = pos * mBlockSize;
        ocall_ReadFile(mFileId, bytePosition,pBlock, mBlockSize, &byteReads);
    }

    return byteReads;
}

int BlockFile::writeBlock(char* pBlock, int pos) {

    int status = 0;
    if (mEncryptionEnabled) {
        uint8_t* output_block = (uint8_t*) malloc(mBlockSize + CMAC_SIZE);
        uint8_t* protected_block = (uint8_t*) malloc(mBlockSize + CMAC_SIZE); // with MAC
        // integrity checking
        uint8_t* cmac_tag = (uint8_t*) malloc(CMAC_SIZE);
        // LOG("1\n");
        compute_CMAC((uint8_t*)pBlock, mBlockSize, cmac_tag);
        memcpy(protected_block, cmac_tag, CMAC_SIZE);
        memcpy(protected_block + CMAC_SIZE, pBlock, mBlockSize);
        // LOG("CMAC-TAG #### %0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x%0x\n", cmac_tag[0],cmac_tag[1],cmac_tag[2],cmac_tag[3],cmac_tag[4],
        // cmac_tag[5],cmac_tag[6], cmac_tag[7],cmac_tag[8],cmac_tag[9],cmac_tag[10],
        // cmac_tag[11],cmac_tag[12],cmac_tag[13],cmac_tag[14],cmac_tag[15]);

        aes_encrypt((uint8_t*) protected_block, output_block, mBlockSize + CMAC_SIZE);

        int bytePosition = pos * (mBlockSize + CMAC_SIZE);
        // LOG("2\n");
        // LOG("pos: %d size: %d block size: %d\n", bytePosition, sizeof(output_block), mBlockSize);
        ocall_WriteFile(mFileId, bytePosition, (char*)output_block, mBlockSize + CMAC_SIZE, &status);
        // LOG("3 free\n");
        free(protected_block);
        free(output_block);
        free(cmac_tag);
    } else {
        // LOG("4\n");
        int bytePosition = pos * mBlockSize;
        ocall_WriteFile(mFileId, bytePosition, pBlock, mBlockSize, &status);
        // LOG("5\n");
    }
    return status;
}

int BlockFile::closeFile() {
    // destructor will close the file.
    ocall_CloseFile(mFileId);
    mFileId = 0;
    return 0;
}

int BlockFile::getBlockHeaderSize() {
        return BlockFile::headerSize;
}
