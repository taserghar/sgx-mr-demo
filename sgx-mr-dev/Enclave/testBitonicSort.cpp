#include "BitonicSort.h"

int main() {

    string outPath = "data/test";
    // string fileName = "data/output/map.out";
    string fileName = "data/test/synthetic_1.txt";
    int blockSize = 1024;
    int recordLimit = 15;
    int NOB = 3000;
    BitonicSort<StringWritable,StringWritable> bs(NOB, fileName, outPath, blockSize, recordLimit);
    // bs.printRecordsFromFile(fileName, blockSize, recordLimit);
    string outFile = bs.sort();
    // printf("----SORT DONE----- output - filename:%s\n", outFile.c_str());
    bs.printRecordsFromFile(outFile, blockSize, recordLimit);
    return 0;
}
