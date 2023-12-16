#include "utils.h"
// #include "BlockFile.h"
// #include "RecordStream.h"
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


// template<class K, class V>
// void printRecordsFromFile(string inputFileName, int blockSize, int recordLimit) {
//     LOG("\n");
//     BlockFile bf(inputFileName, IOMode_READ, blockSize, recordLimit);
//     int iter = 0;
//     char* block = new char[blockSize];
//     vector<Record<K,V>>recordList;
//     while (bf.readBlock(block, iter++) != 0){
//         printf("==> BLOCK # %d\n", iter);
//         RecordInputStream<K,V> ris(block);
//         Record<K,V>rec;
//         while(ris.readRecord(rec)) {
//             // recordList.push_back(rec);
//             printf("=> [%s] %s\n", rec.getKey().getValue().c_str(), rec.getValue().getValue().c_str());
//         }
//         printf("\n");
//     }
// }

clock_t getCurrentTime(){
    return clock();
}

//Justin
double getCurrentTimeInMS(){
	return (double)(clock()) / (CLOCKS_PER_SEC/1000.0);
}

double timeDiff(clock_t prev) {
    return (double)(clock() - prev) / (CLOCKS_PER_SEC/1000.0);
}

double timeDiff(clock_t prior_event, clock_t later_event) {
    return (double)(later_event - prior_event) / (CLOCKS_PER_SEC/1000.0);
}
