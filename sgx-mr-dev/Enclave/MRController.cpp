#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "MRController.h"
#include "BitonicSort.h"
#include "utils.h"
#include "Enclave.h"
#include "MergeSort.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include "sgx_tcrypto.h"
#include "sgx_trts.h"
#include <stdio.h> /* vsnprintf */
#include "BlockOperations.h"
#include "oasm_lib.h"
#include <map>
#include "DemoLog.h" //Justin
//#include <fstream>
#include <string>//Justin
// #include "blockSort.h"
// #include "blockSort.cpp"
using namespace std;

// ---------------- IMapper Class implementation ----------
template  <class K1, class V1, class K2, class V2>
void IMapper<K1,V1,K2,V2>::write(K2 key, V2 value) {
    Record<K2,V2>rec(key,value);
    // LOG("MAP_WRITE_RECORD\n");
    if (recordOutListSize + rec.getSize() < bf.getBlockSize() && (int) recordOutList.size() < bf.getRecordLimit()) {
        recordOutList.push_back(rec);
    } else {
        // LOG("Stream Full! Writing Blocks\n");
        //sorint records before Writing
        if (isObliviousSort) {
            // LOG("Oblivious sort\n");
            obliviousSort(recordOutList);
        } else {
            // LOG("STL SOrt\n");
            std::sort(recordOutList.begin(), recordOutList.end());
        }

	/*
        // it's ok to leak partial group size, no information leak
	if(isObliviousSort)
	{
 	    int map_out_record_count = recordOutList.size();
            map<K2, vector<V2>>my_map;
            V2 val;
       	    for(Record<K2,V2> mr : recordOutList) {
                val = mr.getValue();
                my_map[mr.getKey()].push_back(val);
            }
        int combined_record_count = my_map.size();
        // LOG("combiner: %d %d\n", map_out_record_count, combined_record_count);
        recordOutList.clear();
        // map<K2, int>::iterator it;
        for(auto r: my_map) {
             Record<K2,V2> combinedRecord = combine(r.first, r.second);
             recordOutList.push_back(combinedRecord);
            // LOG("%s: %d\n", r.first.getValue().c_str(), r.second.size());
        }
	}
	*/



        int ret = output.writeRecord(recordOutList);
        if ( ret != (int) recordOutList.size()) {
            ERR("Error: size: %d, written %d\n", (int) recordOutList.size() ,ret);
            return;
        }
        //success
        char* block = output.getBlock();
        // LOG("Writing BLock %d\n", currentOutBlockPos);
        bf.writeBlock(block,currentOutBlockPos++);
        // LOG("MAP_WRITE_BLOCK\n");
        // if(currentOutBlockPos% 10000 == 0) LOG("Map write block# %d\n", currentOutBlockPos);
        blockCounter++;
        //NOTE: don't need to free, block for entire map cycle
        // free(block);
        recordOutList.clear();
        recordOutListSize = BlockFile::getBlockHeaderSize() + 4 ;//
        // considering one record mustbe lesser than record size
        recordOutList.push_back(rec);

    }
    recordOutListSize += rec.getSize();
}

template  <class K1, class V1, class K2, class V2>
void IMapper<K1,V1,K2,V2>::flushRecords() {
    if (recordOutList.size() > 0) {
        if (isObliviousSort) {
            obliviousSort(recordOutList);
        } else {
            std::sort(recordOutList.begin(), recordOutList.end());
        }

        // /////////////////////////
        // to get combiner size, below code up to comment can be removed
        int map_out_record_count = recordOutList.size();
        map<K2, vector<V2>>my_map;
        V2 val;
        for(Record<K2,V2> mr : recordOutList) {
            val = mr.getValue();
            my_map[mr.getKey()].push_back(val);
        }
        int combined_record_count = my_map.size();
        // LOG("combiner: %d %d\n", map_out_record_count, combined_record_count);
        recordOutList.clear();
        // map<K2, int>::iterator it;
        for(auto r: my_map) {
             Record<K2,V2> combinedRecord = combine(r.first, r.second);
             recordOutList.push_back(combinedRecord);
            // LOG("%s: %d\n", r.first.getValue().c_str(), r.second.size());
        }

        // while( recordOutList.size()  < bf.getRecordLimit()) {
        //     recordOutList.push_back(Record<K2,V2>(k2,v2));
        // }
        // end analysis, aboce code up t ocomment can be removed
        // /////////////////////////

        int ret = output.writeRecord(recordOutList);
        if ( ret != (int) recordOutList.size()) {
            ERR("Error: size: %d, written %d\n", (int) recordOutList.size() ,ret);

        }
        //success
        char* block = output.getBlock();
        // LOG("Writing BLock %d\n", currentOutBlockPos);
        bf.writeBlock(block,currentOutBlockPos++);
        // LOG("MAP_WRITE_BLOCK\n");
        blockCounter++;
        delete [] block;
        recordOutList.clear();
        bf.closeFile();
    }
    // LOG("|||||||||||||||||| Mapper Output Total Blocks: %d\n",blockCounter);
}

template  <class K1, class V1, class K2, class V2>
void IMapper<K1,V1,K2,V2>::setOutput(string outFile, int pBlockSize, int pRecordLimit) {
    // outFile = "../data/map.out";
    bf = BlockFile(outFile, IOMode_WRITE, pBlockSize, pRecordLimit, true);
    output = RecordOutputStream<K2,V2>(pBlockSize, pRecordLimit);
    currentOutBlockPos = 0;
}


template  <class K1, class V1, class K2, class V2>
vector<pair<K2,V2>>& IMapper<K1,V1,K2,V2>::getMapperOutput(){
    return output;
}

template  <class K1, class V1, class K2, class V2>
void IMapper<K1,V1,K2,V2>::printMapOutput(){

}

template  <class K1, class V1, class K2, class V2>
void IMapper<K1,V1,K2,V2>::simObliviousSwap(Record<K2,V2>&a, Record<K2,V2>&b, int cond) {
    if (cond) {
        Record<K2,V2> dummy = a;
        a = b;
        b = dummy;
    } else {
        Record<K2,V2> dummy = a;
        a = b;
        b = a;
        a = dummy;
    }
}

template  <class K1, class V1, class K2, class V2>
void IMapper<K1,V1,K2,V2>::obliviousSwap(Record<K2,V2>&a, Record<K2,V2>&b, int cond) {
    int recordSize = bf.mBlockSize/bf.mRecordLimit;
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

template  <class K1, class V1, class K2, class V2>
void IMapper<K1,V1,K2,V2>::obliviousSort(vector<Record<K2,V2>>& array) {
    int i, j, k;
    int store_n = array.size();
    int N;
    Record<K2,V2> dummy;
    for( N = store_n; N & (N - 1); N++) {
        array.push_back(dummy);
    }

    for (k=2;k<=N;k=2*k) {
        // LOG("group with %d blocks\n", k);
        for (j=k>>1;j>0;j=j>>1) {
            for (i=0;i<N;i++) {

                int ixj=i^j;
                if ((ixj)>i) {
                    if ((i&k)==0) {
                        //ascending
                        // compAndSwap(inputBF,inputBF, i, ixj, 1);
                        if (enable_oblivious_swap) {
                            int cond =array[ixj] < array[i] ;
                            obliviousSwap(array[ixj], array[i],cond);
                        } else {
                            if ( array[ixj] < array[i] ) swap(array[ixj], array[i]);
                        }

                    }
                    if ((i&k)!=0) {
                        //descending
                        if (enable_oblivious_swap) {
                            int cond = array[i] < array[ixj] ;
                            obliviousSwap(array[ixj], array[i],cond);
                        } else {
                            if ( array[i] < array[ixj] ) swap(array[ixj], array[i]);
                        }

                    }
                    // LOG("Back to main algorithm\n");
                }
            }
        }
    }
    while(array.size()!=store_n) array.pop_back();
}

template  <class K2, class V2, class K3, class V3>
void IReducer<K2,V2,K3,V3>::write(Record<K3,V3>& rec) {
    if (!output.writeRecord(rec)) {
        // LOG("Stream Full! Writing Blocks\n");
        char* block = output.getBlock();
        // printBlockAsRecord(block);
        reducerBF.writeBlock(block,currentOutBlockPos++);
        // LOG("REDUCE_WRITE_BLOCK\n");
        // don't need to free because it's for entire reduce cycle
        // delete[]
        output.resetBlock();
        //re-initializing block
        // output = RecordOutputStream<StringWritable,StringWritable>(reducerBF.getBlockSize(), reducerBF.getRecordLimit());
        output.writeRecord(rec);
    }
}

template  <class K2, class V2, class K3, class V3>
void IReducer<K2,V2,K3,V3>::flushWriter() {
    if(!output.isEmpty()) {
        Record<K3,V3> rec;
        output.writeRecord(rec);
        char* block = output.getBlock();
        // printBlockAsRecord(block);
        reducerBF.writeBlock(block, currentOutBlockPos++);
        // LOG("REDUCE_WRITE_BLOCK\n");
        output.resetBlock();
        // LOG("May be Need Fix flushWriter\n");
       delete[] block;
    }
}

template  <class K2, class V2, class K3, class V3>
void IReducer<K2,V2,K3,V3>::setOutput(string outFile, int pBlockSize, int pRecordLimit) {
    reducerBF = BlockFile(outFile, IOMode_WRITE, pBlockSize, pRecordLimit, true);
    output = RecordOutputStream<K3,V3>(pBlockSize, pRecordLimit);
    currentOutBlockPos = 0;
}

template  <class K2, class V2, class K3, class V3>
void IReducer<K2,V2,K3,V3>::printReduceOutput(){

}

//------------------- Job Class methods


template <class K1, class V1, class K2, class V2, class K3, class V3>
Job<K1,V1,K2,V2,K3,V3>::Job(){
    // combiner = new Combiner<K2,V2>();
}

template <class K1, class V1, class K2, class V2, class K3, class V3>
void Job<K1,V1,K2,V2,K3,V3>::setInputFiles(vector<string>inFiles) {
    inputFiles = inFiles;
}
template <class K1, class V1, class K2, class V2, class K3, class V3>
void Job<K1,V1,K2,V2,K3,V3>::setOutputPath(string outPath) {
    outputPath = outPath;
}


template <class K1, class V1, class K2, class V2, class K3, class V3>
void Job<K1,V1,K2,V2,K3,V3>::setMapper(IMapper<K1,V1,K2,V2>* _mapper) {
    mapper = _mapper;
}

template <class K1, class V1, class K2, class V2, class K3, class V3>
void Job<K1,V1,K2,V2,K3,V3>::setReducer(IReducer<K2,V2,K3,V3>* _reducer) {
    reducer = _reducer;
}

template <class K1, class V1, class K2, class V2, class K3, class V3>
void Job<K1,V1,K2,V2,K3,V3>::setBlockSize(int pBlockSize) {
    mBlockSize = pBlockSize;
}

template <class K1, class V1, class K2, class V2, class K3, class V3>
void Job<K1,V1,K2,V2,K3,V3>::setBlockRecordLimit(int pRecordLimit) {
    mBlockRecordLimit = pRecordLimit;
}

// return the index
template<class K, class V>
int getAllValueByKey(K key, vector<Record<K,V>>& list, int i, vector<V>& outList ) {
    for( i = i; i < (int) list.size(); i++) {
        if(list[i].getKey() != key) break;
        // LOG("REDUCE_READ_RECORD\n");
        outList.push_back(list[i].getValue());
    }
    return i;
}

// dummy Reducer
template<class K, class V>
Record<K,V> dummyReduce(K key, vector<V>& valueList) {
    int res = 0;
    // dummy for word count;
    for(auto v: valueList) {
        int convertedInt =  atoi(v.getValue().c_str());
        res = res + convertedInt;
    }
    StringWritable value(StringWritable(to_string(res)));
    return Record<K,V>(key, value);
}

template <class K1, class V1, class K2, class V2, class K3, class V3>
int Job<K1,V1,K2,V2,K3,V3>::executeMapPhase(string mapOutFilename) {
    mapper->setup();
    mapper->setOutput(mapOutFilename, mBlockSize, mBlockRecordLimit);
    mapper->blockCounter = 0;
    mapper->recordOutListSize = BlockFile::getBlockHeaderSize() + 4; // TODO: update 4 to constant val
     
    //logfile.open("logs.txt");
    //if(logfile.is_open())
	    //LOG("LOGFILE SUCCESS\n");


    for( auto fileName: inputFiles) {
         LOG("Reading block file %s\n", fileName.c_str());
        BlockFile bf(fileName, IOMode_READ, mBlockSize, mBlockRecordLimit, true);
        char* block = new char[mBlockSize];
        for( int ind = 0;; ind++) {
            //reading all blocks in file
            if(bf.readBlock(block, ind)!=0) {
	        if(ind % 20 == 0)
		{
			std::string midMsg = "Processing record " + std::to_string(ind) + ":";
			DemoLog::updateClient(midMsg);
		}
                 // LOG("MAP_READ_BLOCK\n");
                vector<Record<K1,V1>>recordList;
                RecordInputStream<K1,V1>ris(block);
                //reading records from stream
                ris.readRecord(recordList);
                 // LOG("reading blocks..%d.\n", ind);
                for( auto& r: recordList) {
                    // LOG("MAP_READ_RECORD\n");
                    //LOG("Value: %d\n", r.getValue());
                    mapper->mapp(r.getKey(), r.getValue());
                }
            } else {
                //END OF FILE
                break;
            }
        }

        bf.closeFile();
        // freeing the block memory
        delete[] block;
    }
    mapper->flushRecords();
    // LOG("Mapper JOB [DONE]\n");
    // LOG("MAP OUTPUT TOTAL %d BLOCK\n",  mapper->blockCounter);
    //returning number of block after map task
    return mapper->blockCounter;
}

template <class K1, class V1, class K2, class V2, class K3, class V3>
int Job<K1,V1,K2,V2,K3,V3>::executeSortPhase(string mapOutFilename, int noOfBlockinMap, int sort_type) {
    // LOG("Sorting Mapped Output");
    // clock_t initTime = getCurrentTime();

    BlockFile mappedBF = BlockFile(mapOutFilename, IOMode_READ, mBlockSize, mBlockRecordLimit, true);
    switch (sort_type) {
        case sort_type_bitonic_sort:
        {
            LOG("Bitonic Sort Enabled\n");
            BitonicSort<StringWritable,StringWritable> bs(noOfBlockinMap, (BlockOperations*)&mappedBF, mapOutFilename, mBlockSize, mBlockRecordLimit); 
            noOfBlockinMap = bs.sort();
            // bs.printRecordsFromFile(noOfBlockinMap, mBlockSize, mBlockRecordLimit);
            mappedBF.closeFile();
            // LOG("Mapped Output Sort Complete\n");
            break;
        }
        case sort_type_merge_sort:
        {
            LOG("Merge Sort Enabled\n");
            BlockFile tempMemory_w = BlockFile(mapOutFilename + ".temp", IOMode_WRITE, mBlockSize, mBlockRecordLimit, true);//last arge true
            tempMemory_w.closeFile();
            BlockFile tempMemory_r = BlockFile(mapOutFilename + ".temp", IOMode_READ, mBlockSize, mBlockRecordLimit, true);//last arg true
	    LOG("BEFORE SORTING\n");
            MergeSort<StringWritable,StringWritable> ms(noOfBlockinMap, mBlockSize, mBlockRecordLimit,  &mappedBF, &tempMemory_r);
            ms.sort();
	    LOG("SORTING FINISHED CLOSING FILES\n");
            // MergeSort(int pNob, int pBlockSize, int pRecordLimit, BlockFile* dataMemory, BlockFile* tempMemory);
            tempMemory_r.closeFile();
            mappedBF.closeFile();
            break;
        }
        default:
            ERR("No Valid Sort Selected\n");
    }
    return noOfBlockinMap;

}

template <class K1, class V1, class K2, class V2, class K3, class V3>
void Job<K1,V1,K2,V2,K3,V3>::executeReducePhase(string mapOutFilename, string reduceOutFilename, int noOfBlockinMap, bool enablePadding){
    // taking sorted output  as input
    BlockFile bf(mapOutFilename, IOMode_READ, mBlockSize, mBlockRecordLimit, true);
    char* block = new char[mBlockSize];
    Record<K2,V2>prevRecord;
    int blockIndex = 0;

    int gs = 0;//no significance on reduce
    reducer->setOutput(reduceOutFilename, mBlockSize, mBlockRecordLimit);
    // LOG("Total block to reduce: %d\n", noOfBlockinMap);
    // TODO: last block is making segmentation fault for KMEANS but WC is working perfectly
    //TODO: Check in future
    while( blockIndex < noOfBlockinMap - 1 && bf.readBlock(block, blockIndex)!=0) {
        // LOG("REDUCE_READ_BLOCK\n");
        RecordInputStream<K2,V2>ris(block);
        vector<Record<K2,V2>>recordList;
        // LOG("**************** check point 1\n");
        //reading records from stream
        int ind = 0;
        ris.readRecord(recordList);
        // LOG("**************** check point 1.1\n");
        // for( Record<K2,V2> record: recordList) {
        //     LOG("Reduced Record: %s\n", record.getKey().getValue().c_str());
        // }
        if(blockIndex == 0) {
            prevRecord = recordList[0];
            ind = 1;
        }

        // LOG("Reducing Start for block# %d\n", blockIndex);
        while( ind < (int) recordList.size()) {
            vector<V2>valueList;
            ind = getAllValueByKey(prevRecord.getKey(), recordList, ind, valueList );
            // adding value of prevRecord for hirerchical reduce
            valueList.push_back(prevRecord.getValue());
            gs = gs + valueList.size();
            // LOG("**************** check point 3\n");
            Record<K3,V3>reducedRecord = reducer->reduce(prevRecord.getKey(), valueList);
            // LOG("reduce function computed\n");
            if( ind < (int) recordList.size()) {
                // TODO: write the reduced Record to buffer and if buffer full wirte it to file

                // StringWritable tk = reducedRecord.getKey();
                // StringWritable tv =(StringWritable)reducedRecord.getValue();
                // LOG("r %s\n", tk.getValue().c_str());
                // LOG("before writing..\n");
                reducer->write(reducedRecord);
                 // printf("%d\n", gs);
                 gs = 0;
                prevRecord = recordList[ind];
                // LOG("REDUCE_READ_RECORD\n");
                ind++;
            } else{
                // LOG("add for previous\n");
                prevRecord = reducedRecord;
            }
        }

        if (enablePadding && blockIndex >= reducer->currentOutBlockPos) {
            //TODO: need to write proper dummy blocks with meda data for padding
            // currently using without meta data
            memset(block, 0, mBlockSize);
            reducer->reducerBF.writeBlock(block,reducer->currentOutBlockPos++);
        }
        blockIndex++;

    }
     // printf("%d\n", gs);
    // LOG("BlockCount: %d\n", reducer->currentOutBlockPos);
    delete[] block;

    
    // TODO: prevRecord is the last reducedRecord need to write in the file.
    reducer->write(prevRecord);
    reducer->flushWriter();
    reducer->reducerBF.closeFile();
}



template <class K1, class V1, class K2, class V2, class K3, class V3>
void Job<K1,V1,K2,V2,K3,V3>::run(bool enablePadding,int sort_type){ 
    
    DemoLog::updateClient("SGXMR_START");	
    string mapOutFilename = outputPath + "map.out";
    string reduceOutFilename = outputPath + "reduce.out";
    mapper->isObliviousSort = sort_type == sort_type_bitonic_sort;

    LOG("Mapping...\n");
    ocall_takeTimestamp();
    int mappedBlockCount = executeMapPhase(mapOutFilename);
    DemoLog::updateClient("MAP_FINISHED");

    LOG("Sorting...\n");
    ocall_takeTimestamp();
    mappedBlockCount = executeSortPhase(mapOutFilename, mappedBlockCount, sort_type);
    DemoLog::updateClient("SORT_FINISHED");

    LOG("Reducing...\n");
    ocall_takeTimestamp();
    executeReducePhase(mapOutFilename, reduceOutFilename, mappedBlockCount, enablePadding);
    DemoLog::updateClient("REDUCE_FINISHED");
    ocall_takeTimestamp();

  

    // deallocating
    delete mapper;
    delete reducer;
}
