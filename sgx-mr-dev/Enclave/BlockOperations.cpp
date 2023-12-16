
#include "BlockOperations.h"
#include "BlockFile.h"

class BlockFileCopy {
public:
    BlockFile bf;
    BlockFileCopy(string pFileName, IOMode_t pIOMode, int pBlockSize, int pRecordLimit, bool pEncryptionEnabled){
         bf = BlockFile(pFileName, pIOMode, pBlockSize, pRecordLimit, pEncryptionEnabled);
    }

    int read (char* pblock, int pos ) {
        return bf.readBlock(pblock, pos);
    }
    void closeFile(){
        bf.closeFile();
    }
};

// SimpleBlockFile Implementation
InEnclaveBlockFile::InEnclaveBlockFile(){}
InEnclaveBlockFile::InEnclaveBlockFile(string pFileName, IOMode_t pIOMode, int pBlockSize, int pRecordLimit, bool pEncryptionEnabled) {
    mBlockSize = pBlockSize;
    BlockFileCopy bfc(pFileName, pIOMode, pBlockSize, pRecordLimit, pEncryptionEnabled);
    char* block = new char[mBlockSize];
    int blockIndex = 0;
    while(bfc.read(block, blockIndex++)!=0) {
        for( int i = 0; i < mBlockSize; i++) fileBuf.push_back(block[i]);
    }
    bfc.closeFile();
    //TODO: need to re-open and write back when everything is done
}

int InEnclaveBlockFile::readBlock(char* pBlock, int pos) {
	int ret;
    pos = pos * mBlockSize;
    if( pos + mBlockSize <= (int) fileBuf.size()) {
        std::copy(fileBuf.begin() + pos, fileBuf.begin() + pos + mBlockSize, pBlock);
        ret = mBlockSize;
    } else {
        // LOG("returning zero\n");
        ret = 0;
    }
    return ret;

}

int InEnclaveBlockFile::writeBlock(char* pBlock, int pos) {

	int ret;
    pos = pos * mBlockSize;
    if( pos + mBlockSize > (int) fileBuf.size()) {
        fileBuf.resize(pos + mBlockSize);
    }
    std::copy(pBlock, pBlock + mBlockSize, fileBuf.begin() + pos);
    ret = 0;
    return ret;
}

// int InEnclaveBlockFile::getTotalBlockCount() {
// 	return maxBlock;
// }

int InEnclaveBlockFile::getBlockSize(){
	return mBlockSize;
}

// void InEnclaveBlockFile::destroyMemory() {
// 	uint8_t ret;
// 	// destroyMemoryBlocks(&ret);
// }
