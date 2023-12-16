#include "synthetic_data.h"

using namespace std;

static int recordLimit = 15;
static int recordSize = 20;
//Justin tried changing these but it didn't help with mergesort
static int blockSize = 1024;
static int NOB = 3000;


Record<StringWritable,StringWritable> getSyntheticRecord() {

    string key;
    string val;
    for( int i =0 ; i < recordSize; i++) {
        key.push_back(getRandomNumber()%26 + 'A');
    }

    for( int i =0 ; i < recordSize; i++) {
        val.push_back(getRandomNumber()%26 + 'a');
    }
    LOG("Converting to string Writable\n");
    StringWritable K(key);
    StringWritable V(val);
    LOG("Converting to Record\n");
    Record<StringWritable,StringWritable>rec(K,V);
    LOG("return\n");
    return rec;
}
 void convTextToRecord( string outputFile) {

     int blockCnt = 0;
     BlockFile outputBF = BlockFile(outputFile, IOMode_WRITE, blockSize, recordLimit, false);
     vector<Record<StringWritable,StringWritable>> records;

     for( int ind = 0; ind < NOB; ind++) {
         // for( int r_ind = 0; r_ind < recordLimit; r_ind++) {
         //     records.push_back(getSyntheticRecord());
         //     LOG("record id: %d\n", r_ind);
         // }
         // LOG("1\n");
         // sort(records.begin(), records.end());
         LOG("2\n");
         RecordOutputStream<StringWritable, StringWritable>outStream(blockSize, recordLimit);
         LOG("3\n");
         // outStream.writeRecord(records);
         LOG("4\n");
         char* block = outStream.getBlock();
         LOG("writing block\n");
         outputBF.writeBlock(block, blockCnt++);
         LOG("5\n");
         delete[] block;
         records.clear();
     }
     outputBF.closeFile();
 }


 void testRecordInputStream(string inputFile) {



     BlockFile bf(inputFile, IOMode_READ, blockSize, recordLimit, true);
     int iter = 0;
     int bytesRead = -1;
     char* block = new char[blockSize];
     vector<Record<StringWritable,StringWritable>>recordList;
     while (bf.readBlock(block, iter++) != 0){
         printf("############## bytesRead: %d\n", bytesRead);
         printf("[Block]");
         // printBytes(block, blockSize);
         RecordInputStream<StringWritable,StringWritable> ris(block);
         Record<StringWritable,StringWritable>rec;
         while(ris.readRecord(rec)) {
             recordList.push_back(rec);
         }
     }

     for( auto& r:recordList) {
         printf("=> %s %s\n", r.getKey().getValue().c_str(), r.getValue().getValue().c_str());
     }
 }


 void printRecordsFromFile(string inputFileName, int blockSize, int recordLimit) {
     LOG("\n");
     BlockFile bf(inputFileName, IOMode_READ, blockSize, recordLimit, true);
     int iter = 0;
     char* block = new char[blockSize];
     vector<Record<StringWritable,StringWritable>>recordList;
     while (bf.readBlock(block, iter++) != 0){
         printf("==> BLOCK # %d\n", iter);
         RecordInputStream<StringWritable,StringWritable> ris(block);
         Record<StringWritable,StringWritable>rec;
         while(ris.readRecord(rec)) {
             // recordList.push_back(rec);
             printf("=> [%s] %s\n", rec.getKey().getValue().c_str(), rec.getValue().getValue().c_str());
         }
         printf("---[%s]---\n", "GAP");
     }
 }

void gen_synthetic(int p_nob, int p_blockSize, int p_recordSize, int p_recordLimit ) {
     NOB = p_nob;
     blockSize = p_blockSize;
     recordSize = p_recordSize;
     recordLimit = p_recordLimit;

     string pre = "../data/test/";
     string post = "sort_data.txt";
     convTextToRecord(pre + post);
     // convTextToRecord(inputFile, outputFile);
     // testRecordInputStream(outputFile);
 }
