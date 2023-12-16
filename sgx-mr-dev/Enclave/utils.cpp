#include "utils.h"
#include "BlockFile.h"
#include "RecordStream.h"
#include <ctime>


int min(int x, int y) {
    return (x<y)? x:y;
}
// must contain exactly 4 byte
int bytes_to_int(char* bytes){
    int B0 = 0xFF & (bytes[0]);
    int B1 = 0xFFFF & (bytes[1] << 8);
    int B2 = 0xFFFFFF & (bytes[2] << 16);
    int B3 = 0xFFFFFFFF & (bytes[3] << 24);
    int value =  0xFFFFFFFF & (B0 | B1 | B2 | B3);
    return value;
}
// bytes must be allocated to size 4 or more
void int_to_bytes(int value, char* bytes){
    int size = 4;
    for (int i = 0; i < size; i++) {
        bytes[i] = 0xFF&(value >> (i * 8));
    }
}


template<class K, class V>
void printRecordsFromFile(string inputFileName, int blockSize, int recordLimit) {
    LOG("\n");
    BlockFile bf(inputFileName, IOMode_READ, blockSize, recordLimit);
    int iter = 0;
    char* block = new char[blockSize];
    vector<Record<K,V>>recordList;
    while (bf.readBlock(block, iter++) != 0){
        printf("==> BLOCK # %d\n", iter);
        RecordInputStream<K,V> ris(block);
        Record<K,V>rec;
        while(ris.readRecord(rec)) {
            // recordList.push_back(rec);
            printf("=> [%s] %s\n", rec.getKey().getValue().c_str(), rec.getValue().getValue().c_str());
        }
        printf("\n");
    }
}

unsigned int getRandomNumber(){
   unsigned int r;
   sgx_read_rand((unsigned char*) &r, 4);
   return r;
}

// Bitonic Sort: Work directly wit number of records
// void obliviousSort(vector<int>& array) {
//     int i, j, k;
//     int store_n = array.size();
//     int N;
//     int dummy = 9999999;
//     for( N = store_n; N & (N - 1); N++) {
//         array.push_back(dummy);
//     }
//
//     for (k=2;k<=N;k=2*k) {
//         // LOG("group with %d blocks\n", k);
//         for (j=k>>1;j>0;j=j>>1) {
//             for (i=0;i<N;i++) {
//
//                 int ixj=i^j;
//                 if ((ixj)>i) {
//                     if ((i&k)==0) {
//                         //ascending
//                         // compAndSwap(inputBF,inputBF, i, ixj, 1);
//                         if ( array[ixj] < array[i] ) swap(array[ixj], array[i]);
//                     }
//                     if ((i&k)!=0) {
//                         //descending
//                         if ( array[i] < array[ixj] ) swap(array[ixj], array[i]);
//                     }
//                     // LOG("Back to main algorithm\n");
//                 }
//             }
//         }
//     }
//     while(array.size()!=store_n) array.pop_back();
// }
