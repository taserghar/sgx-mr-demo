#include "RecordStream.h"
#include "BlockFile.h"

// Record Read Stream ----

template<class K, class V>
RecordInputStream<K,V>::RecordInputStream(char* pBlock) {
    mBlock = pBlock;
    iter = BlockFile::getBlockHeaderSize();
    mBlockSize = bytes_to_int(pBlock);
    iter+=4; // TODO: add constant of intsize to header
    //LOG("In RIS constructor--mBlockSize: %d\n", mBlockSize);
}

template<class K, class V>
bool RecordInputStream<K,V>::readRecord(Record<K,V>& pRecord) {
    bool status = false;
    if (iter < mBlockSize) {
        iter+= pRecord.read(mBlock + iter);	
        status = true;
    }
    return status;
}

template<class K, class V>
int RecordInputStream<K,V>::readRecord(vector<Record<K,V>>& pRecordList) {
    int cnt = 0;
    while (iter < mBlockSize) {
        Record<K,V> r;
	//LOG("RIS mBlockSize: %d\n", mBlockSize);
	//LOG("RIS iter: %d\n", iter);
        iter+= r.read(mBlock + iter);
	//LOG("finished read at index %d\n", iter);
	pRecordList.push_back(r);
        cnt++;
    }
    return cnt;
}
// Record Write Stream -----

template<class K, class V>
RecordOutputStream<K,V>::RecordOutputStream() {}

template<class K, class V>
RecordOutputStream<K,V>::~RecordOutputStream() {
    // if (mBlock) delete[] mBlock;
}

template<class K, class V>
RecordOutputStream<K,V>::RecordOutputStream(int pBlockSize, int pRecordLimit) {
    mBlock = new char[pBlockSize];
    memset(mBlock, 0, pBlockSize);
    mBlockSize = pBlockSize;
    mRecordLimit = pRecordLimit;
    currentTotalRecord = 0;
    iter = BlockFile::getBlockHeaderSize() + 4; // TODO: replace hardcoded value
}

template<class K, class V>
void RecordOutputStream<K,V>::setBlock(char* pBlock, int pBlockSize) {
    mBlock = pBlock;
    mBlockSize = pBlockSize;
    iter = BlockFile::getBlockHeaderSize() + 4;
    currentTotalRecord = 0;
}

template<class K, class V>
char* RecordOutputStream<K,V>::getBlock() {
    int headerSize = 0;//BlockFile::getBlockHeaderSize();
    int_to_bytes(iter - headerSize - 4, mBlock + headerSize);
    return mBlock;
}

template<class K, class V>
void RecordOutputStream<K,V>::resetBlock() {
    // if (mBlock) delete[] mBlock;
    // mBlock = new char[mBlockSize];
    currentTotalRecord = 0;
    iter = BlockFile::getBlockHeaderSize() + 4; // TODO: replace hardcoded value
}

template<class K, class V>
bool RecordOutputStream<K,V>::isEmpty() {
    bool status = false;
    if (iter == BlockFile::getBlockHeaderSize() + 4) {
        status = true;
    }
    return status;
}

template<class K, class V>
bool RecordOutputStream<K,V>::writeRecord(Record<K,V>& pRecord) {
    bool status = false;
    if (currentTotalRecord < mRecordLimit && iter + pRecord.getSize() < mBlockSize) {
        iter+= pRecord.write( mBlock + iter);
        currentTotalRecord++;
        status = true;
    }
    return status;
}

template<class K, class V>
int RecordOutputStream<K,V>::writeRecord(vector<Record<K,V>>& pRecordList) {
    int ind = 0;
    iter = BlockFile::getBlockHeaderSize() + 4; // TODO:: need to update the iter appropriately for single record
    currentTotalRecord = 0;
    while (ind < (int)pRecordList.size() && (iter + pRecordList[ind].getSize() < mBlockSize && currentTotalRecord < mRecordLimit)) {
        Record<K,V> r(pRecordList[ind]);
        iter+= r.write( mBlock + iter);
        currentTotalRecord++;
        ind++;

    }
    return ind;
}



template class RecordInputStream<StringWritable,Point>;
template class RecordOutputStream<StringWritable,Point>;

template class RecordInputStream<StringWritable,StringWritable>;
template class RecordOutputStream<StringWritable,StringWritable>;


// template class RecordInputStream<DBKey,StringWritable>;
// template class RecordOutputStream<DBKey,StringWritable>;
