#include "MergeSort.h"
#include "BlockFile.h"
#include "Enclave.h"
#include "utils.h"
#include "oasm_lib.h"
using namespace std;

template<class K, class V>
MergeSort<K,V>::MergeSort(int pNob, int pBlockSize, int pRecordLimit, BlockFile* data, BlockFile* addMemory) {
    nob = pNob; // no. of blocks
    dataMemory  = data;
    additionalmemory = addMemory;
    blockSize = pBlockSize; // block size
    recordLimit = pRecordLimit;
}

template<class K, class V>
void MergeSort<K,V>::readRecords(vector<Record<K,V>>& records, char* block){
    // printf("%s()\n", __func__);
    // printBlock(block);

    RecordInputStream<K,V>ris(block);
    //LOG("will read record\n");
    ris.readRecord(records);
    //for( auto& r:records) {
        //printf("=----------------readRecords-----------------------------------------------> %s %s\n", r.getKey().getValue().c_str(), r.getValue().getValue().c_str());
    //}
    //LOG("after read record\n");
    //reversing for easy parsing from back side
    reverse(records.begin(), records.end());
    // printf("~%s() %u\n", __func__, records.size());
}

template<class K, class V>
void MergeSort<K,V>::writeRecord(BlockFile* dataStore,
                RecordOutputStream<K,V>& output,
                Record<K,V>& rec) {
    // printf("%s()\n", __func__);
    if (!output.writeRecord(rec)) {
        // LOG("Stream Full! Writing Blocks\n");
        char* block = (char*) output.getBlock();
        // printBlockAsRecord(block);
        // LOG("will write to block\n");
        dataStore->writeBlock(block, blockCounter++);
        // delete [] block;
        output.resetBlock();
        //re-initializing block
        // output = RecordOutputStream<K,V>(blockSize, recordLimit);
        output.writeRecord(rec);
    }
    // printf("~%s()\n", __func__);
}

template<class K, class V>
void MergeSort<K,V>::flushWriter(BlockFile* dataStore,
                RecordOutputStream<K,V>& output){
    if(!output.isEmpty()) {
        Record<K,V> rec;
        output.writeRecord(rec);
        char* block = (char*)output.getBlock();
        // blockCounter++;
        // printBlockAsRecord(block);
        dataStore->writeBlock(block, blockCounter++);
        output.resetBlock();
        // printf("May be Need Fix flushWriter\n");
       delete[] block;
    }
}

template<class K, class V>
void MergeSort<K,V>::sort() {
    // LOG("\n");
    mergeSort(nob);
}

template<class K, class V>
void MergeSort<K,V>::mergeSort(int n) {
   int currSize;
   int leftStart;
    //LOG("Block Count: %d\n", n);
   for (currSize=1; currSize<=n; currSize = 2*currSize) {
       // string outFileName = getNewFileName();
       //LOG("OUTER BEFORE INNER: curSiz: %d\n", currSize);
       blockCounter = 0;
       for (leftStart=0; leftStart<n; leftStart += 2*currSize) {
           int mid = min(leftStart + currSize - 1, n-1);
           int rightEnd = min(leftStart + 2*currSize - 1, n-1);
           //LOG("INNER: left,mid,right[%d %d %d]\n", leftStart, mid, rightEnd);
           if (enable_oblivious_swap) {
                obliviousBlockMerge(leftStart, mid, rightEnd);
           } else {
                merge(leftStart, mid, rightEnd);
           }

	   //LOG("INNER FOR LOOP\n");
           // flushWriter(outBF, output);
           // printf("Flush Writer end\n");
       }
       //LOG("OUTER AFTER INNER\n");
       // swap(dataMemory,additionalmemory);
       // n = blockCounter;
   }
   LOG("FINISHING MERGESORT\n");
}

template<class K, class V>
void MergeSort<K,V>::obliviousRecordSwap(Record<K,V>&a, Record<K,V>&b, int cond) {
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

//finding order
template<class K, class V>
bool MergeSort<K,V>::isDesc(vector<Record<K,V>>& recordList) {
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
void MergeSort<K,V>::obliviousRecordMerge(vector<Record<K,V>>& records_i,
                    vector<Record<K,V>>& records_j, int dir) {
    vector<Record<K,V>> mergedRecords;
    int size_i = records_i.size();
    int size_j = records_j.size();

    //both size must be equal
    int size_ext;
    // converting to bitonic order
    if (isDesc(records_i)) {
        reverse(records_i.begin(), records_i.end());
    }

    size_ext = size_i;
    //constructor set to max by-default
    K key;
    K val;
    //feeling up to next 2th power
    for( size_ext; size_ext & (size_ext - 1); size_ext++) {
        records_i.push_back(Record<K,V>(key,val));
    }

    // //for records_j
    if (isDesc(records_j)) reverse(records_j.begin(), records_j.end());

    size_ext = size_j;

    //feeling up to next 2th power
    for( size_ext; size_ext & (size_ext - 1); size_ext++) {
        records_j.push_back(Record<K,V>(key,val));
    }
    // 2nd half should be in descending order
    reverse(records_j.begin(), records_j.end());

    //feeling merge with both half
    for(Record<K,V>r : records_i) mergedRecords.push_back(r);
    for(Record<K,V>r : records_j) mergedRecords.push_back(r);
    records_i.clear();
    records_j.clear();

    // oblivious merging
    int k = size_ext * 2;
    for (int j=k >> 1;j>0;j=j>>1) {
        for (int i=0;i<k;i++) {

            int ixj=i^j;
            if ((ixj)>i) {
                if ((i&k)==0) {
                    //ascending
                    if (enable_oblivious_swap) {
                        int cond = mergedRecords[ixj] < mergedRecords[i];
                        obliviousRecordSwap(mergedRecords[i],mergedRecords[ixj],cond);
                    } else {
                        if( mergedRecords[ixj] < mergedRecords[i]) swap(mergedRecords[i],mergedRecords[ixj]);
                    }

                }
                if ((i&k)!=0) {
                    //descending
                    if (enable_oblivious_swap) {
                        int cond = mergedRecords[i] < mergedRecords[ixj];
                        obliviousRecordSwap(mergedRecords[i], mergedRecords[ixj],cond);
                    } else {
                        if( mergedRecords[i] < mergedRecords[ixj]) swap(mergedRecords[i],mergedRecords[ixj]);
                    }

                }
                // LOG("Back to main algorithm\n");
            }
        }
    }

    while(mergedRecords.size() > size_i + size_j) mergedRecords.pop_back();


    if (!dir) reverse(mergedRecords.begin(), mergedRecords.end());

    if( (int) mergedRecords.size() < recordLimit) {
        // LOG("EXIT (1) record limit is larger than merged record ERRORRRR %d", (int)mergedRecords.size());
        return;
    }
    for( int k = 0; k < recordLimit; k++) records_i.push_back(mergedRecords[k]);
    for( int k = recordLimit; k < (int) mergedRecords.size(); k++) records_j.push_back(mergedRecords[k]);
}


template<class K, class V>
void MergeSort<K,V>::obliviousBlockMerge(int l, int m, int r) {
    // LOG("----------------------------------------------------------Merge(): L %d M %d R %d\n", l,m,r);
    int i = l;
    int j = m + 1;
    if ( m + 1 > r) {
        // only left part nothing to be merged
        // this comes when number of elements is not power of 2
        // LOG(" no change\n");
        // char* block = new char[blockSize];
        // for( int i =l; i <=r; i++) {
        //     additionalmemory->readBlock(block, i);
        //     printBlockAsRecord(block);
        // }
        // delete[] block;
        return;
    }
    vector<Record<K,V>>leftRecords;
    vector<Record<K,V>>rightRecords;
    char* leftBlock = new char[blockSize];
    char* rightBlock = new char[blockSize];

    Record<K,V> lRecord;
    Record<K,V> rRecord;
    int wrtCnt = i;
    // LOG("reading Left Block [%d]\n", i);
    dataMemory->readBlock(leftBlock, i);
    readRecords(leftRecords, leftBlock);
    // LOG("reading Right Block[%d]\n", j);
    dataMemory->readBlock(rightBlock, j);
    readRecords(rightRecords, rightBlock);
    // LOG("....... reading inital record complete\n");
    i++;
    j++;
    lRecord = leftRecords.front();
    rRecord = rightRecords.front();

   int isNextLeft = 0;
   int nextIndex;

   // TODO:: What if no right segment exist? or no left segment exist??????????
   while (true) {
       // LOG(" [ %d ] [ %d ]\n", i, j);


        //merging records, ascending = 1
        // LOG("Oblivious Merge\n");
        obliviousRecordMerge(leftRecords, rightRecords, 1);

        RecordOutputStream<K,V>outputStream(blockSize, recordLimit);
        outputStream.writeRecord(leftRecords);
        char* block_temp = outputStream.getBlock();
        additionalmemory->writeBlock(block_temp, wrtCnt++);
        // LOG("delete temp buff\n");
        delete[] block_temp;
        leftRecords.clear();


        // terminating condition
        if (i > m && j > r) {
            // LOG("terminating merge...\n");
            // merging complete
            RecordOutputStream<K,V>outputStreamRight(blockSize, recordLimit);
            outputStreamRight.writeRecord(rightRecords);
            char* block_temp_right = outputStreamRight.getBlock();
            additionalmemory->writeBlock(block_temp_right, wrtCnt++);
            delete[] block_temp_right;
            rightRecords.clear();
            break;
        }
        //finding nextBlock
        if (i > m) {
            isNextLeft = 0;
            nextIndex = j++;
        } else if ( j > r ) {
            isNextLeft = 1;
            nextIndex = i++;
        } else if ( lRecord < rRecord ) {
            isNextLeft = 1;
            nextIndex = i++;
        } else {
            isNextLeft = 0;
            nextIndex = j++;
        }

        // read nextBlock
        // since writing left block, always taking next input in leftBlock
        // LOG("nextIndex: %d\n", nextIndex);
        dataMemory->readBlock(leftBlock, nextIndex);
        readRecords(leftRecords, leftBlock);

        // printBlockAsRecord(leftBlock);
        // LOG("Assigning lRecord or rRecord\n");
        if(isNextLeft) {
            lRecord = leftRecords.front();
            // LOG("updated: left: %s\n", lRecord.getKey().getValue().c_str());
        } else {
            rRecord = leftRecords.front();
                // LOG("updated: right: %s\n", rRecord.getKey().getValue().c_str());
        }
    }



    // Important*************
    // static design very bad, might need change
    // copy back the records from temp memory to main storage
    // but computationally it is negligible as complexity not increased
    for( int i =l; i <=r; i++) {
        additionalmemory->readBlock(rightBlock, i);
        // LOG("block[%d]\n", i);
        // printBlockAsRecord(rightBlock);
        dataMemory->writeBlock(rightBlock, i);
    }
    delete[] leftBlock;
    // delete[] rightBlock;
}

template<class K, class V>
void MergeSort<K,V>::merge(int l, int m, int r) {
    //LOG("---------------------------------------------Merge(): L %d M %d R %d\n", l,m,r);
    int i = l;
    int j = m + 1;

    RecordOutputStream<K,V>outputStream(blockSize, recordLimit);


    vector<Record<K,V>>leftRecords;
    vector<Record<K,V>>rightRecords;
    char* leftBlock = new char[blockSize];
    char* rightBlock = new char[blockSize];

    // LOG("Left-Right Merging Block Start...\n");
    while (i <= m && j <= r) {
	
        if (leftRecords.size() == 0) {
            dataMemory->readBlock(leftBlock, i);
	    //LOG("reading Left Block %d\n", i);
            readRecords(leftRecords, leftBlock);

        }

        if (rightRecords.size() == 0) {
            dataMemory->readBlock(rightBlock, j);
	   
            //LOG("reading Right Block %d\n", j);
	    	
            readRecords(rightRecords, rightBlock);
        }

        //LOG("Merging... both %u %u\n", leftRecords.size(), rightRecords.size());
        while (leftRecords.size() > 0 && rightRecords.size() > 0) {
            // LOG("Merging both start\n");
            Record<K,V>lRecord = leftRecords.back();
            Record<K,V>rRecord = rightRecords.back();

            Record<K,V>outRecord;

            if (lRecord < rRecord) {
                outRecord = lRecord;
                leftRecords.pop_back();
            } else {
                outRecord = rRecord;
                rightRecords.pop_back();
            }
            // LOG("write_record\n");
            writeRecord(additionalmemory, outputStream, outRecord);



            // LOG("Merging both end\n");
        }

        if (leftRecords.size() == 0) i++;
        if (rightRecords.size() == 0) j++;
    }

    // handling remaining records from left segment
     // LOG("handling remaining records from left segment\n");
    while (i <= m) {
        if (leftRecords.size() == 0) {
            dataMemory->readBlock(leftBlock, i);
            // LOG("reading Left Block[%d]\n", i);
            readRecords(leftRecords, leftBlock);
        }
        while (leftRecords.size() > 0) {
            writeRecord(additionalmemory, outputStream, leftRecords.back());
            leftRecords.pop_back();
        }
        i++; // moving to next block
    }

    // handling remaining records from right segment
    // printf("handling remaining records from right segmen\n");
    while (j <= r) {
        if (rightRecords.size() == 0) {
            dataMemory->readBlock(rightBlock, j);
            // LOG("reading Right Block[%d]\n", j);
            readRecords(rightRecords, rightBlock);
        }
        while (rightRecords.size() > 0) {
            writeRecord(additionalmemory, outputStream, rightRecords.back());



            rightRecords.pop_back();
        }
        j++; // moving to next block
    }

    flushWriter(additionalmemory,outputStream);

    // Important*************
    // static design very bad, might need change
    // copy back the records from temp memory to main storage
    // but computationally it is negligible as complexity not increased
    for( int i =l; i <=r; i++) {
        additionalmemory->readBlock(rightBlock, i);
        // printBlockAsRecord(rightBlock);
        dataMemory->writeBlock(rightBlock, i);
    }
    delete[] leftBlock;
    delete[] rightBlock;
}

template<class K, class V>
void MergeSort<K,V>::printRecords(vector<Record<K,V>>& records) {
    for( int i =0; i < (int) records.size(); i++) {
        Record<K,V>r = records[i];
        LOG("## (%s,%s)\n", r.getKey().getValue().c_str(), r.getValue().getValue().c_str());
    }
}

template<class K, class V>
void MergeSort<K,V>::printRecord(Record<K,V>&r) {
    printf("(%s,%s) size: %d\n", r.getKey().getValue().c_str(), r.getValue().getValue().c_str(), r.getSize());
}

template<class K, class V>
void MergeSort<K,V>::printBlock(char* block) {
    printf("Printing Blocks:\n");
    // for( int i =0; i < blockSize; i++) {
    //     printf("%0X ", block[i]);
    // }
    // printf("\n");
    for( int i =0; i < blockSize; i++) {
        printf("%c ", block[i]);
    }
    printf("\n");
}


template<class K, class V>
void MergeSort<K,V>::printBlockAsRecord(char* block) {
    LOG("\n");
    RecordInputStream<K,V> ris(block);
    Record<K,V>rec;
    while(ris.readRecord(rec)) {
        // recordList.push_back(rec);
        LOG("=> [%s] %s\n", rec.getKey().getValue().c_str(), rec.getValue().getValue().c_str());
    }
    LOG("\n");
}

// void printRecordsFromFile(string inputFileName, int blockSize, int recordLimit) {
//
//     // BlockFile bf(inputFileName, IOMode_READ, blockSize, recordLimit);
//     // int iter = 0;
//     // char* block = new char[blockSize];
//     // vector<Record<K,V>>recordList;
//     // while (bf.readBlock(block, iter++) != 0){
//     //     RecordInputStream<K,V> ris(block);
//     //     Record<K,V>rec;
//     //     while(ris.readRecord(rec)) {
//     //         // recordList.push_back(rec);
//     //         printf("=> [%s] %s\n", rec.getKey().getValue().c_str(), rec.getValue().getValue().c_str());
//     //     }
//     //     printf("\n");
//     //     //if( iter == 6) break;
//     // }
//
//     // for( auto& r:recordList) {
//     //     printf("=> [%s] %s\n", r.getKey().getValue().c_str(), r.getValue().getValue().c_str());
//     // }
// }

template class MergeSort<StringWritable,StringWritable>;
