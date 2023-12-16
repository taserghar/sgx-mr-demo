#include "BitonicSort.h"
#include "oasm_lib.h"


using namespace std;

template<class K, class V>
BitonicSort<K,V>::BitonicSort(int pNob, BlockOperations* inputBF, string pPath, int pBlockSize, int pRecordLimit) {

    nob = pNob; // no. of blocks
    // fileName = pFileName; // input filename
    bf = inputBF;
    outPath = pPath;
    blockSize = pBlockSize; // block size
    recordLimit = pRecordLimit;
    // LOG("**** nob:%d, blocksize: %d, recordlimit: %d\n", nob, pBlockSize, pRecordLimit);

}

template<class K, class V>
void BitonicSort<K,V>::preprocess() {
    //filling blocks up to 2^n
    // string outFileName = getNewFileName();
    BlockOperations* inputBF = bf;
    // BlockFile outputBF = BlockFile(outFileName, IOMode_WRITE, blockSize, recordLimit);
    char* tBlock = new char [blockSize];
    // re-writing up to nob - 1 blocks
    int cur_out_block = 0;
    vector<Record<K,V>>inRecords;
    vector<Record<K,V>>outRecords;

    for( int i =0 ; i < nob; i++) {
        inputBF->readBlock(tBlock, i);
        inRecords.clear();
        readRecords(inRecords, tBlock);
        // LOG("Prev block size(#record): %u\n", inRecords.size());
        for( int j =0; j < inRecords.size(); j++) {
            outRecords.push_back(inRecords[j]);
            if( outRecords.size() == recordLimit) {
                RecordOutputStream<K,V> ros(blockSize, recordLimit);
                ros.writeRecord(outRecords);
                // LOG("New block size(#record): %u\n", outRecords.size());
                tBlock = ros.getBlock();
                inputBF->writeBlock(tBlock, cur_out_block++);
                // LOG("Block written: %d\n", cur_out_block);
                // TODO: Check if not delete tBlock, will create memory leaks
                // delete[] tBlock;

                outRecords.clear();
            }
        }
        //pushed the records to into blocks and remove emmpty solots
    }

    // // last block might not be up to record limit per block
    // K key;
    // V val;
    // // reading last block
    // inputBF->read    // // last block might not be up to record limit per block
    K key;
    V val;
    // // reading last block
    // inputBF->readBlock(tBlock, nob - 1);
    vector<Record<K,V>>tRecords;
    // readRecords(tRecords, tBlock);
    // delete[] tBlock;
    // lastBlockRecordCount = tRecords.size();
    // // filling last block with dummy records
    // for( int i = (int) tRecords.size(); i < recordLimit; i++) {
    //     tRecords.push_back(Record<K,V>(key,val));
    // }Block(tBlock, nob - 1);
    // vector<Record<K,V>>tRecords;
    // readRecords(tRecords, tBlock);
    // delete[] tBlock;
    // lastBlockRecordCount = tRecords.size();
    // filling last block with dummy records

    lastBlockRecordCount = outRecords.size();
    for( int i = (int) outRecords.size(); i < recordLimit; i++) {
        outRecords.push_back(Record<K,V>(key,val));
    }
    RecordOutputStream<K,V> ros(blockSize, recordLimit);
    ros.writeRecord(outRecords);

    tBlock = ros.getBlock();
    inputBF->writeBlock(tBlock, cur_out_block++);
    delete[] tBlock;
    tRecords.clear();
    nob = cur_out_block;
    // LOG("New Block Count: %d\n", nob);
    vector<Record<K,V>>dummyRecords;

    for( int i = 0; i < recordLimit; i++) {
        dummyRecords.push_back(Record<K,V>(key,val));
    }
    RecordOutputStream<K,V> ros_dummy(blockSize, recordLimit);
    // LOG("record limit: %d, size: %u", recordLimit, dummyRecords.size());
    ros_dummy.writeRecord(dummyRecords);

    char* dummyBlock;
    dummyBlock = ros_dummy.getBlock();
    for( nobExtended = nob; nobExtended & (nobExtended - 1); nobExtended++) {
        inputBF->writeBlock(dummyBlock, nobExtended);
    }
    dummyRecords.clear();
    delete[] dummyBlock;
}

template<class K, class V>
string BitonicSort<K,V>::postProcess()  {
    // string outFileName = getNewFileName();
    BlockOperations* inputBF = bf;//BlockFile(fileName, IOMode_READ, blockSize, recordLimit);
    // BlockFile outputBF = BlockFile(outFileName, IOMode_WRITE, blockSize, recordLimit);

    char* tBlock = new char [blockSize];
    // for( int i =0 ; i < nob -1; i++) {
    //     inputBF->readBlock(tBlock, i);
    //     inputBF->writeBlock(tBlock, i);
    // }
    //writing last block, removing dummy records;
    // LOG("reading last block: index %d\n", nob - 1);
    inputBF->readBlock(tBlock, nob-1);
    vector<Record<K,V>>tRecords;
    readRecords(tRecords, tBlock);

    delete[] tBlock;
    int extraRecord = recordLimit - lastBlockRecordCount;
    while(extraRecord--) tRecords.pop_back();

    // LOG("removed Extra records: original size: %d, current size: %d\n", recordLimit, lastBlockRecordCount);
    RecordOutputStream<K,V> ros(blockSize, recordLimit);
    ros.writeRecord(tRecords);

    tBlock = ros.getBlock();
    //re-writing last block after removing dummy records
    inputBF->writeBlock(tBlock, nob-1);


    delete[] tBlock;

    // inputBF.closeFile();
    return fileName;
}


template<class K, class V>
string BitonicSort<K,V>::getNewFileName() {
    static int fileCounter = 1;
    string name = outPath + "/blockfile_" + to_string(fileCounter);
    fileCounter++;
    return name;
}

template<class K, class V>
void BitonicSort<K,V>::readRecords(vector<Record<K,V>>& records, char* block){
    // printBlock(block);

    if( block == NULL) {
        LOG("block NULL\n");
    }
    RecordInputStream<K,V>ris(block);
    ris.readRecord(records);
}

template<class K, class V>
int BitonicSort<K,V>::sort() {
    // LOG("\n");
    preprocess();
    bitonicIterative(nobExtended);
    postProcess();
    return nob;
}


//finding order
template<class K, class V>
bool BitonicSort<K,V>::isDesc(vector<Record<K,V>>& recordList) {
    size_t siz = recordList.size();
    for( int i  = 1; i < siz; i++) {
        if ( recordList[i] < recordList[i-1] ) {
            // found descending
            return true;
        } else if( recordList[i-1] < recordList[i]) {
            // already ascending
            return false;
        }
    }
    return false;
}

// mergeRecords for comaparing blockwise records
template<class K, class V>
void BitonicSort<K,V>::mergeRecords(vector<Record<K,V>>& records_i,
                    vector<Record<K,V>>& records_j, int dir) {
    vector<Record<K,V>> mergedRecords;
    int size_i = records_i.size();
    int size_j = records_j.size();

    // reversing when in descending order
    if (isDesc(records_i)) reverse(records_i.begin(), records_i.end());
    if (isDesc(records_j)) reverse(records_j.begin(), records_j.end());


    int ind_i = 0, ind_j = 0;

    while(ind_i < size_i && ind_j < size_j) {
        K key_i = records_i[ind_i].getKey();
        K key_j = records_j[ind_j].getKey();

        if( records_i[ind_i] < records_j[ind_j]) {
            mergedRecords.push_back(records_i[ind_i++]);
        } else {
            mergedRecords.push_back(records_j[ind_j++]);
        }
    }

    while( ind_i < size_i) mergedRecords.push_back(records_i[ind_i++]);
    while( ind_j < size_j) mergedRecords.push_back(records_j[ind_j++]);

    records_i.clear();
    records_j.clear();

    if (!dir) reverse(mergedRecords.begin(), mergedRecords.end());

    if( (int) mergedRecords.size() < recordLimit) {
        LOG("EXIT (1) record limit is larger than merged record ERRORRRR %d", (int)mergedRecords.size());
        return;
    }
    for( int k = 0; k < recordLimit; k++) records_i.push_back(mergedRecords[k]);
    for( int k = recordLimit; k < (int) mergedRecords.size(); k++) records_j.push_back(mergedRecords[k]);
}

template<class K, class V>
void BitonicSort<K,V>::simObliviousSwap(Record<K,V>&a, Record<K,V>&b, int cond) {
    if (cond) {
        Record<K,V> dummy = a;
        a = b;
        b = dummy;
    } else {
        Record<K,V> dummy = a;
        a = b;
        b = a;
        a = dummy;
    }
}

template<class K, class V>
void BitonicSort<K,V>::obliviousSwap(Record<K,V>&a, Record<K,V>&b, int cond) {
    int recordSize = blockSize/recordLimit;
    // LOG("Record Size: %d\n", recordSize);
    char* buff_a = new char[recordSize];
    char* buff_b = new char[recordSize];
    a.write(buff_a);
    b.write(buff_b);

    oswap_buffer((unsigned char*)buff_a, (unsigned char*)buff_b, recordSize, cond);
    a.read(buff_a);
    b.read(buff_b);
    delete[] buff_a;
    delete[] buff_b;
}

// mergeRecords for comaparing blockwise records
template<class K, class V>
void BitonicSort<K,V>::obliviousMerge(vector<Record<K,V>>& records_i,
                    vector<Record<K,V>>& records_j, int dir) {
    vector<Record<K,V>> mergedRecords;
    int size_i = records_i.size();
    int size_j = records_j.size();
    // LOG("1\n");
    //both size must be equal
    int size_ext;
    // converting to bitonic order
    if (isDesc(records_i)) {
        reverse(records_i.begin(), records_i.end());
    }
    // LOG("2\n");
    size_ext = size_i;
    //constructor set to max by-default
    K key;
    K val;
    //feeling up to next 2th power
    for( size_ext; size_ext & (size_ext - 1); size_ext++) {
        records_i.push_back(Record<K,V>(key,val));
    }
    // LOG("3\n");
    //for records_j
    if (isDesc(records_j)) reverse(records_j.begin(), records_j.end());
    // LOG("4\n");
    size_ext = size_j;

    //feeling up to next 2th power
    for( size_ext; size_ext & (size_ext - 1); size_ext++) {
        records_j.push_back(Record<K,V>(key,val));
    }
    // 2nd half should be in descending order
    reverse(records_j.begin(), records_j.end());
    // LOG("5\n");
    //feeling merge with both half
    for(Record<K,V>r : records_i) mergedRecords.push_back(r);
    for(Record<K,V>r : records_j) mergedRecords.push_back(r);
    // LOG("merged records size: %u\n", mergedRecords.size());
    records_i.clear();
    records_j.clear();
    // LOG("6\n");
    // oblivious merging
    int k = size_ext * 2;
    for (int j=k >> 1;j>0;j=j>>1) {
        for (int i=0;i<k;i++) {
            // LOG("6.1.a\n");
            int ixj=i^j;
            if ((ixj)>i) {
                // LOG("6.1.b\n");
                if ((i&k)==0) {
                    // LOG("ascending.. compare record %d %d\n", ixj, i);
                    //ascending
                    if (enable_oblivious_swap) {
                        int cond = mergedRecords[ixj] < mergedRecords[i];
                        // LOG("6.1\n");
                        obliviousSwap(mergedRecords[i],mergedRecords[ixj],cond);
                    } else {
                        if( mergedRecords[ixj] < mergedRecords[i]) swap(mergedRecords[i],mergedRecords[ixj]);
                    }
                    // LOG("6.2\n");
                }
                // LOG("6.2.a\n");
                if ((i&k)!=0) {
                    // LOG("descending..\n");
                    //descending
                    // LOG("6.3\n");
                    if (enable_oblivious_swap) {
                        int cond = mergedRecords[i] < mergedRecords[ixj];
                        obliviousSwap(mergedRecords[i], mergedRecords[ixj],cond);
                    } else {
                        if( mergedRecords[i] < mergedRecords[ixj]) swap(mergedRecords[i],mergedRecords[ixj]);
                    }
                    // LOG("6.4\n");
                }
                 // LOG("Back to main algorithm\n");
            }
            // LOG("6.5\n");
        }
        // LOG("6.6\n");
    }
    // LOG("7\n");
    while(mergedRecords.size() > size_i + size_j) mergedRecords.pop_back();


    if (!dir) reverse(mergedRecords.begin(), mergedRecords.end());

    if( (int) mergedRecords.size() < recordLimit) {
        LOG("EXIT (1) record limit is larger than merged record ERRORRRR %d", (int)mergedRecords.size());
        return;
    }

    LOG("8\n");
    for( int k = 0; k < recordLimit; k++) records_i.push_back(mergedRecords[k]);
    for( int k = recordLimit; k < (int) mergedRecords.size(); k++) records_j.push_back(mergedRecords[k]);
}

// Compare function for blocks
// i, j block position in file
template<class K, class V>
void BitonicSort<K,V>::compAndSwap(BlockOperations* outFile, BlockOperations* inputFile, int i, int j, int dir) {

    // take the inputs from i and j blocks
    // merge them in sorted order.
    char* block_i = new char[blockSize];
    char* block_j = new char[blockSize];
    inputFile->readBlock(block_i, i);
    inputFile->readBlock(block_j, j);
    vector<Record<K,V>> records_i;
    vector<Record<K,V>> records_j;
    // LOG("Read block: %d\n", i);
    readRecords(records_i, block_i);
    // LOG("Read block: %d\n", j);
    readRecords(records_j, block_j);
    delete[] block_i;
    delete[] block_j;
    //TODO: Need to optimize by storing blocks previous order (asc/desc)
    //finding directions
    // LOG("merging block %d & %d\n", i, j);
    mergeRecords(records_i, records_j, dir);
    // obliviousMerge(records_i, records_j, dir);
    // LOG("******OUTPUT******* -- %d-> %d              %d-> %d\n", i, records_i.size(), j, records_j.size());
    RecordOutputStream<K,V>outStream_i(blockSize, recordLimit);
    RecordOutputStream<K,V>outStream_j(blockSize, recordLimit);
    outStream_i.writeRecord(records_i);
    outStream_j.writeRecord(records_j);
    block_i = outStream_i.getBlock();
    block_j = outStream_j.getBlock();

    inputFile->writeBlock(block_i, i);
    inputFile->writeBlock(block_j, j);
    // LOG("block written\n");
    delete[] block_i;
    delete[] block_j;
    // LOG("deleted block\n");
}

template<class K, class V>
void BitonicSort<K,V>::bitonicIterative(int N ){
    int i,j,k;
    // LOG("Total Block: %d\n", N);
    BlockOperations* inputBF = bf;//BlockFile(fileName, IOMode_READ, blockSize, recordLimit);
    for (k=2;k<=N;k=2*k) {
        // LOG("group with %d blocks\n", k);
        for (j=k>>1;j>0;j=j>>1) {
            for (i=0;i<N;i++) {

                int ixj=i^j;
                if ((ixj)>i) {
                    if ((i&k)==0) {
                        //ascending
                        compAndSwap(inputBF,inputBF, i, ixj, 1);
                    }
                    if ((i&k)!=0) {
                        //descending
                        compAndSwap(inputBF,inputBF, i, ixj, 0);
                    }
                    // LOG("Back to main algorithm\n");
                }
            }
        }
    }
}

template<class K, class V>
void BitonicSort<K,V>::printRecords(vector<Record<K,V>>& records) {
    //
    // for( int i =0; i < (int) records.size(); i++) {
    //     Record<K,V>r = records[i];
    //     printf("(%s,%s)\n", r.getKey().getValue().c_str(), r.getValue().getValue().c_str());
    // }
}

template<class K, class V>
void BitonicSort<K,V>::printRecord(Record<K,V>&r) {
    // printf("(%s,%s) size: %d\n", r.getKey().getValue().c_str(), r.getValue().getValue().c_str(), r.getSize());
}

template<class K, class V>
void BitonicSort<K,V>::printBlock(char* block) {
    // printf("Printing Blocks:\n");
    // for( int i =0; i < blockSize; i++) {
    //     printf("%0X ", 0xFF&block[i]);
    // }
    // printf("\n");
    // for( int i =0; i < blockSize; i++) {
    //     printf("%c ", block[i]);
    // }
    // printf("\n");
}

template<class K, class V>
void BitonicSort<K,V>::printBlockAsRecord(char* block) {
    // LOG("\n");
    // RecordInputStream<K,V> ris(block);
    // Record<K,V>rec;
    // LOG("Start Reading Record\n");
    // while(ris.readRecord(rec)) {
    //     // recordList.push_back(rec);
    //     printf("=> [%s] %s\n", rec.getKey().getValue().c_str(), rec.getValue().getValue().c_str());
    // }
    // printf("\n");
}
template<class K, class V>
void BitonicSort<K,V>::printRecordsFromFile(int nob, int blockSize, int recordLimit) {
    LOG("\n");

    int iter = 0;
    char* block = new char[blockSize];
    vector<Record<K,V>>recordList;
    while ( iter < nob ){
        bf->readBlock(block, iter++);
        printf("==> BLOCK # %d\n", iter);
        RecordInputStream<K,V> ris(block);
        Record<K,V>rec;
        while(ris.readRecord(rec)) {
            // recordList.push_back(rec);
            printf("=> [%s] %s\n", rec.getKey().getValue().c_str(), rec.getValue().getValue().c_str());
        }
        printf("%c%c\n", ' ', ' ');
    }
}

template class BitonicSort<StringWritable,StringWritable>;
