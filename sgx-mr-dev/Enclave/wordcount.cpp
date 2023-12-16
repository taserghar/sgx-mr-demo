//Author AKM Mubashwir Alam

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>
#include <map>
#include "Writable.h"
#include "RecordStream.h"
#include "BlockFile.h"
#include "MRController.h"
#include "MRController.cpp"
#include "utils.h"
#include "Enclave.h"

using namespace std;


class MyMapper : public IMapper<StringWritable,StringWritable,StringWritable,StringWritable> {
public:
    void setup(){

    }

    void mapp(StringWritable key, StringWritable value){
         // LOG("Called\n");
        string line = value.getValue();
        // if(stoi(key.getValue()) >= 177683) LOG("LINE: %s\n", line.c_str());
        for( size_t i = 0; i < line.size(); i++){
            size_t j = i;
            string word;
            while(j < line.size() && !isalpha(line[j])) j++;
            while(j < line.size() && isalpha(line[j++])){
                word.push_back(line[j-1]);

            }
            i = j - 1;
            StringWritable wordWritable(word);
            StringWritable one("1");
            write(word, one);
            // if(stoi(key.getValue()) >= 177683) LOG("*** Word: %s\n", word.c_str());
        }
    }

    Record<StringWritable,StringWritable> combine(StringWritable key, vector<StringWritable>& values) {
        int res = 0;
        for(auto v: values) {
            res += atoi(v.getValue().c_str());
        }
        StringWritable value(StringWritable(to_string(res)));
        return Record<StringWritable,StringWritable>(key, value);

    }

    int calcSize(){
        return 0;
    }

    bool isSpillRequire(){
        return false;
    }
};


class MyReducer:public IReducer<StringWritable,StringWritable,StringWritable,StringWritable> {
public:

    void setup(){

    }

    Record<StringWritable,StringWritable> reduce(StringWritable key, vector<StringWritable>& values) {
        int res = 0;
        for(auto v: values) {
            res += atoi(v.getValue().c_str());
        }
        StringWritable value(StringWritable(to_string(res)));
        return Record<StringWritable,StringWritable>(key, value);

    }

    int calcSize(){
        return 0;
    }

    bool isSpillRequire(){
        return false;
    }
};


void wordcount(int blkSize, int mEnablePadding, int mSortType, string inputFile, string outPath) {
    bool enablePadding = mEnablePadding >0? true : false;
    int xTimes = (blkSize + 2048 - 1) / 2048;
    const int BlockSize = blkSize ;//* xTimes;
    //TODO: need to set the record limit carefully.
    const int RecordLimit = 30 * xTimes;
    // LOG("Block Size: %d\n", BlockSize);
    // LOG("Record Limit: %d\n", RecordLimit);

    string inputName = inputFile;
    // inputName = "../data/input/block_data.txt";

    vector<string>inFiles{inputName};

    // string outPath = "../data/output/";
    Job<StringWritable,StringWritable,StringWritable,StringWritable,StringWritable,StringWritable> job;

    job.setInputFiles(inFiles);
    job.setOutputPath(outPath);
    job.setMapper(new MyMapper());
    job.setReducer(new MyReducer());
    job.setBlockSize(BlockSize);
    job.setBlockRecordLimit(RecordLimit);
    job.run(enablePadding, mSortType);
}
