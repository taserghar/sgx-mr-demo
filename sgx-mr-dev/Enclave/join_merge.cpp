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
#include "join_merge.h"
using namespace std;


class MyMapper : public IMapper<StringWritable,StringWritable,StringWritable,StringWritable> {
public:
    void setup(){

    }

    string getNextValue(string str, size_t i) {
        string tname;
        size_t size= str.size();
        for( i = i; i < size; i++) {
            if( str[i] == '-') break;
            tname.push_back(str[i]);
        }
        return tname;
    }

    void mapp(StringWritable key, StringWritable value){
        LOG("mapp invoked\n");
        string line = value.getValue();
        size_t size = line.size();
        // LOG("line: %s\n", line.c_str());
        //parsing rows
        vector<string>rows;
        for( size_t i = 0; i < size; i++) {
            string row;

            while( i < size && line[i]!=',' ) {
                row.push_back(line[i]);
                i++;
            }
            rows.push_back(row);
        }

        for( string r : rows) {
            size_t lcnt = 0;
            string tname = getNextValue(r, lcnt);
            lcnt+=(tname.size() + 1);
            string k = getNextValue(r, lcnt);
            lcnt+=(k.size() + 1);
            string v = getNextValue(r, lcnt);
            lcnt+=(v.size() + 1);
            // LOG("TableName: %s %s %s\n", tname.c_str(),  k.c_str(),  v.c_str());

            StringWritable keyWritable(k + "-" + tname);

            StringWritable valueWritable(r);
            write(keyWritable, valueWritable);
        }
        // StringWritable wordWritable(word);
        //
        // StringWritable one("1");
        // write(word, one);

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
    string lastDeptId = "Default";
    string lastDeptName = "Default";

    string getNextValue(string str, size_t i) {
        string tname;
        size_t size= str.size();
        for( i = i; i < size; i++) {
            if( str[i] == '-') break;
            tname.push_back(str[i]);
        }
        return tname;
    }

    Record<StringWritable,StringWritable> reduce(StringWritable key, vector<StringWritable>& values) {
        int res = 0;
        // LOG("tempVal: %s\n", tempVal.c_str());
        string joined_str;
        for(auto r: values) {

            size_t lcnt = 0;
            string tname = getNextValue(r.getValue(), lcnt);
            lcnt+=(tname.size() + 1);
            string k = getNextValue(r.getValue(), lcnt);
            lcnt+=(k.size() + 1);
            string v = getNextValue(r.getValue(), lcnt);
            lcnt+=(v.size() + 1);
            // LOG(" %s\n", v.getValue().c_str());

            if (tname.find("t1") != std::string::npos) {
                lastDeptId = k; // key is dept_id;
                lastDeptName = v; // v is deptName;
            } else {
                // v is the employee name;
                string joined_record = v + "-" + lastDeptName;
                LOG("joined_record: %s\n", joined_record.c_str());
                joined_str.append(joined_record + ",");
            }
            // res += atoi(v.getValue().c_str());
        }

        StringWritable joinedValue(joined_str);
        StringWritable outKey("dummy");
        return Record<StringWritable,StringWritable>(outKey, joinedValue);

    }

    int calcSize(){
        return 0;
    }

    bool isSpillRequire(){
        return false;
    }
};

void test(){
    LOG("TEST\n");
    StringWritable key_1("myTestKey");
    StringWritable tableName_1("MyTableName");
    StringWritable key_2("myKey_1");
    StringWritable tableName_2("t1");
    DBKey dbKey_1(key_1, tableName_1);
    DBKey dbKey_2(key_2, tableName_2);

    // LOG("test values: %s %s\n",dbKey_2.getKey().getValue().c_str(),dbKey_2.getTableName().getValue().c_str() );

    char* block = new char[100];
    dbKey_1.write(block);

    DBKey dbKey_3;
    dbKey_3.read(block);
    LOG("Read from Block: %s %s\n",dbKey_3.getKey().getValue().c_str(),dbKey_3.getTableName().getValue().c_str() );
}
//
unsigned int drawRandomNumber(){
   unsigned int r;
   sgx_read_rand((unsigned char*) &r, 4);
   return r;
}

string getRandomString(int len) {
    string str;
    for( int j = 0; j < len; j++) {
        str.push_back(drawRandomNumber() % 26 + 'A');
    }
    return str;
}

int xTimes = 1;
const int BlockSize = 2048 * xTimes;
const int RecordLimit = 30 * xTimes;
string table = "../data/input/table.txt";
string table_1 = "../data/input/table_1.txt";
string table_2 = "../data/input/table_2.txt";
string tableOutPath = "../data/output/";

// void create_record(int )
//department
void create_table_1(int blockCount){
    BlockFile bf_t1(table,IOMode_WRITE, BlockSize,RecordLimit,true);

    //department id department name;
    int no_dept = 100;
    int dept_id = 1;
    string line;
    int block_id = 0;
    for( int i = 0; i < no_dept; i++) {
        // dept id, dept name
        line.append("t1-" +to_string( dept_id++) + "-" + "dept0"+to_string(dept_id)+",");

        if( i > 0 && i % RecordLimit == 0) {
            LOG("table-1: %s\n", line.c_str());
            Record<StringWritable,StringWritable>record(StringWritable(to_string(i)), StringWritable(line));
            vector<Record<StringWritable,StringWritable>>recordList;
            recordList.push_back(record);
            RecordOutputStream<StringWritable,StringWritable>ros(BlockSize,RecordLimit);
            ros.writeRecord(recordList);
            char* block = ros.getBlock();
            bf_t1.writeBlock(block, block_id++);
            delete[] block;
            line = "";
        }
    }

    int no_employee = 1000;

    for( int i = 0; i < no_employee; i++) {
        //dept-id, employe-name
        line.append("t2-" + to_string(1 + (drawRandomNumber() % no_dept)) + "-" +  getRandomString(8)+",");

        if( i > 0 &&  i % RecordLimit == 0) {
            LOG("table-2: %s\n", line.c_str());
            Record<StringWritable,StringWritable>record(StringWritable(to_string(i)), StringWritable(line));
            vector<Record<StringWritable,StringWritable>>recordList;
            recordList.push_back(record);
            RecordOutputStream<StringWritable,StringWritable>ros(BlockSize,RecordLimit);
            ros.writeRecord(recordList);
            char* block = ros.getBlock();
            bf_t1.writeBlock(block, block_id++);
            delete[] block;
            line = "";
        }
    }


    LOG("Total Block: %d\n", block_id);
    bf_t1.closeFile();
}

//
void encoder_join(int nob) {
    create_table_1(nob);
}

void join_merge(int mEnablePadding, int mSortType) {

    // bool enablePadding = mEnablePadding >0? true : false;
    // int xTimes = 1;
    // const int BlockSize = 2048 * xTimes;
    // const int RecordLimit = 30 * xTimes;
    // LOG("Block Size: %d\n", BlockSize);
    // LOG("Record Limit: %d\n", RecordLimit);
    //
    // string inputName;
    // inputName = "../data/input/table.txt";
    //
    // vector<string>inFiles{inputName};
    //
    // string outPath = "../data/output/";
    // Job<StringWritable,StringWritable,StringWritable,StringWritable,StringWritable,StringWritable> job;
    //
    // job.setInputFiles(inFiles);
    // job.setOutputPath(outPath);
    // job.setMapper(new MyMapper());
    // job.setReducer(new MyReducer());
    // job.setBlockSize(BlockSize);
    // job.setBlockRecordLimit(RecordLimit);
    // job.run(enablePadding, mSortType);
}
