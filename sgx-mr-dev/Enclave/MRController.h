
#ifndef  __MR_CONTROLLER__
#define __MR_CONTROLLER__
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include "Writable.h"
#include "RecordStream.h"
#include "BlockFile.h"
using namespace std;

const int sort_type_bitonic_sort = 1;
const int sort_type_merge_sort = 2;
// <KeyIn, ValueIn, KeyOut, ValueOut>
template <class K1, class V1, class K2, class V2>
class IMapper {
private:

public:
    bool isObliviousSort;
    int currentOutBlockPos;
    BlockFile bf;
    int blockCounter;
    RecordOutputStream<K2,V2>output;
    vector<Record<K2,V2>>recordOutList;
    int recordOutListSize;
    virtual void setup()=0;
    virtual void mapp( K1 key, V1 value)=0;
    virtual int calcSize()=0;
    //temporary combiner,TODO: required proper combiner
    virtual Record<K2,V2> combine(K2 key, vector<V2>& values)=0;

    void write(K2 key, V2 value);
    void setOutput(string outFile, int blockSize, int pRecordLimit);
    void writeToTempFile();
    vector<pair<K2,V2>>& getMapperOutput();
    void printMapOutput();
    void flushRecords();
    void simObliviousSwap(Record<K2,V2>&a, Record<K2,V2>&b, int cond);
    void obliviousSwap(Record<K2,V2>&a, Record<K2,V2>&b, int cond);
    void obliviousSort(vector<Record<K2,V2>>& array);
};

// template <class K2, class V2>
// class Combiner {
// private:
//     map<K2,vector<V2>>output;
// public:
//     void combine(string filename, int blockSize);
//     map<K2,vector<V2>>& getCombinedOutput();
//     void printCombinedOutput();
// };

template <class K2, class V2, class K3, class V3>
class IReducer {
private:

public:
    int currentOutBlockPos;
    BlockFile reducerBF;
    RecordOutputStream<K2,V2>output;
    virtual void setup()=0;
    virtual Record<K3,V3> reduce(K2 key, vector<V2>& values)=0;
    virtual int calcSize()=0;

    void write(Record<K3,V3>& rec);
    void flushWriter();
    void setOutput(string outFile, int blockSize, int pRecordLimit);
    void writeToTempFile();
    void printReduceOutput();
};

template <class K1,class V1, class K2, class V2, class K3, class V3>
class Job {
public:
    int mBlockSize;
    int mBlockRecordLimit;
    // mapper reducer interfaces
    IMapper<K1,V1,K2,V2>* mapper;
    // Combiner<K2,V2>* combiner;
    IReducer<K2,V2,K3,V3>* reducer;
    // input/output files
    vector<string>inputFiles;
    string outputPath;
    
// public:
    Job();
    void setInputFiles(vector<string>infiles);
    void setOutputPath(string outPath);
    void setMapper(IMapper<K1,V1,K2,V2>* _mapper);
    void setReducer(IReducer<K2,V2,K3,V3>* _reducer);
    void setBlockSize(int pBlockSize);
    void setBlockRecordLimit(int pRecordLimit);
    int executeMapPhase(string filename);
    void executeReducePhase(string mappedFilename, string reducedFileName, int blockCount, bool enablePadding);
    void executeReducePhaseForJoin(string mapOutFilename, string reduceOutFilename, int noOfBlockinMap, bool enablePadding);
    int executeSortPhase(string filename, int blockCount, int sortType);
    void run(bool enablePadding, int sortType);
};



#endif
