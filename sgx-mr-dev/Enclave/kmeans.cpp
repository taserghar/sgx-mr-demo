//Author AKM Mubashwir Alam

#include <cstdio>
#include <limits>
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
#include "kmeans.h"
using namespace std;


void generatePoints(vector<Point>&pts, int cnt) {
    for( int i = 0; i < cnt; i++ ) {
        int x = getRandomNumber() % 100000;
        int y = getRandomNumber() % 100000;
        // LOG("x: %d y:%d\n", x, y);
        Point pt;
        pt.nop = 1;
        pt.coordinates.push_back(x);
        pt.coordinates.push_back(y);
        pts.push_back(pt);
    }
}

void kmeans_encoder(int mTotalBlock, int mBlockSize, string centroidFile, string coordinateFile) {

    // string centroidFile = "../data/input/centroids.txt";
    // string coordinateFile = "../data/input/coordinates.txt";

    // min block size 2 KB
    int xTimes = mBlockSize / 2048;
    const int BlockSize = 2048 * xTimes;
    const int RecordLimit = 30 * xTimes;

    int totalBlock = mTotalBlock;
    int centroidBlockCount = 1;


    BlockFile centroidBF = BlockFile(centroidFile, IOMode_WRITE, BlockSize, RecordLimit, true);
    //generating & writing centroids
    int centroid_curr = 0;
    for( int i =0 ; i < centroidBlockCount; i++) {
        vector<Point>centroids;
        generatePoints(centroids, 5);
        vector<Record<StringWritable,Point>> records;
        for( Point pt : centroids) records.push_back(Record<StringWritable, Point>(to_string(centroid_curr++), pt));

        RecordOutputStream<StringWritable, Point>outStream(BlockSize, RecordLimit);
        outStream.writeRecord(records);
        char* block = outStream.getBlock();
        centroidBF.writeBlock(block, i);
        delete[] block;
        records.clear();

    }
    centroidBF.closeFile();

    LOG("Centroid Block Count: %d Centroid Count: %d\n", centroidBlockCount, centroid_curr);


    BlockFile coordinateBF = BlockFile(coordinateFile, IOMode_WRITE, BlockSize, RecordLimit, true);
    //generating & writing Points for KMeans
    int offset = 0;
    for( int i =0 ; i < totalBlock; i++) {
        vector<Point>centroids;
        generatePoints(centroids, RecordLimit);
        vector<Record<StringWritable,Point>> records;
        for( Point pt : centroids) records.push_back(Record<StringWritable, Point>(to_string(offset++), pt));


        RecordOutputStream<StringWritable, Point>outStream(BlockSize, RecordLimit);
        outStream.writeRecord(records);
        char* block = outStream.getBlock();
        coordinateBF.writeBlock(block, i);
        delete[] block;
        records.clear();

    }
    coordinateBF.closeFile();
    LOG("Total Block: %d Centroid Count: %d\n",totalBlock, offset);

}

void test_point(){
    Point pt_1;
    for( int i =1; i <=2; i++) {
        pt_1.coordinates.push_back(i*i);
    }
    pt_1.nop = 1;

    Point pt_2;
    for( int i =1; i <=2; i++) {
        pt_2.coordinates.push_back(i + 3.5);
    }
    pt_2.nop = 1;

    pt_1.add(pt_2);
    LOG("added: nop: %d\n", pt_1.nop);
    LOG("added: values: %lf %lf\n", pt_1.coordinates[0],pt_1.coordinates[1] );
    LOG("check stringValue: %s\n", pt_1.encode(pt_1));
    pt_1.merge();
    LOG("merge: nop: %d\n", pt_1.nop);
    LOG("merge: values: %lf %lf\n", pt_1.coordinates[0],pt_1.coordinates[1] );

    char* block = new char[100];
    pt_1.write(block);

    Point pt_new;
    pt_new.read(block);

    LOG("serialized: nop: %d\n", pt_new.nop);
    LOG("serialized: values: %lf %lf\n", pt_new.coordinates[0],pt_new.coordinates[1] );

    string key_temp = "key-5";
    Record<StringWritable,Point>r(key_temp, pt_new);
    r.write(block);
    Record<StringWritable,Point>r_new;
    r_new.read(block);
    Point pt_3 = r_new.getValue();
    string v = r_new.getKey().getValue();
    LOG("serialized record: values:  %s %lf %lf\n",v.c_str(), pt_new.coordinates[0],pt_new.coordinates[1] );

}



// #########################################################

class KMeansMapper : public IMapper<StringWritable,Point,StringWritable,Point> {

public:
    int mBlockSize;
    int mRecordLimit;
    vector<Record<StringWritable,Point>>centroids;
    BlockFile* bf;
    void setup(){
        char* block = new char[mBlockSize];
        for( int ind = 0; bf->readBlock(block, ind)!=0 ; ind++) {
                //reading all blocks in file

                vector<Record<StringWritable,Point>>recordList;
                RecordInputStream<StringWritable,Point>ris(block);
                //reading records from stream
                ris.readRecord(recordList);
                centroids.insert(centroids.end(), recordList.begin(), recordList.end());
        }
        delete[] block;
    }

    double findDist(Point pt1, Point pt2) {
            int dim = (int) pt1.coordinates.size();
			double sum=0;
			for (int i=0; i<dim; i++){
                // LOG("coordinate: %lf , %lf\n", pt1.coordinates[i],pt2.coordinates[i] );
				double d = pt1.coordinates[i]-pt2.coordinates[i];
				sum += d*d;
			}
            // LOG("%lf\n", sum);
			return sqrt(sum);
		}


    string findClosestPoint(Point pt) {
        double min_dist =  999999999999;//numeric_limits<double>::max();

        string ret;

        for( auto& r: centroids) {

            string key= r.getKey().getValue();
            Point center = r.getValue();
            double dist = findDist(pt, center);
            if ( dist < min_dist ) {
                min_dist = dist;
                ret = key;
            }
            // LOG("Mapp Point#### :%s # %lf %lf\n", key.c_str(), center.coordinates[0],center.coordinates[1] );
        }
        // LOG("Min Dist: %s -- %lf \n", ret.c_str(), min_dist);
        // returning label of the closest centroid
        return ret;
    }
    void mapp(StringWritable key, Point pt){
         // LOG("Called\n");
        // LOG("Mapp Point#### :%s # %lf %lf\n", key.getValue().c_str(), pt.coordinates[0],pt.coordinates[1] );
        string offset = key.getValue();

        string closest_label = findClosestPoint(pt);
        // LOG("Closest Found\n");
        write(closest_label, pt);
        // LOG("written\n");
    }

    Record<StringWritable,Point> combine(StringWritable key, vector<Point>& values) {
        Point pt;
        pt.coordinates.push_back(0);
        pt.coordinates.push_back(0);

        for(auto v: values) {
            pt.add(v);
        }
         // Record<StringWritable,Point>(key, pt)
        // LOG("Appended :%s # %s\n", key.getValue().c_str(), pt.encode(pt).c_str() );
        return Record<StringWritable,Point>(key, pt);

    }

    int calcSize(){
        return 0;
    }

    bool isSpillRequire(){
        return false;
    }
};


class KmeansReducer:public IReducer<StringWritable,Point,StringWritable,Point> {
public:

    void setup(){
    }

    Record<StringWritable,Point> reduce(StringWritable key, vector<Point>& values) {
        Point pt;
        pt.coordinates.push_back(0);
        pt.coordinates.push_back(0);

        for(auto v: values) {
            pt.add(v);
        }
         // Record<StringWritable,Point>(key, pt)
        // LOG("Appended :%s # %s\n", key.getValue().c_str(), pt.encode(pt).c_str() );
        return Record<StringWritable,Point>(key, pt);

    }

    int calcSize(){
        return 0;
    }

    bool isSpillRequire(){
        return false;
    }
};



// void test_sort(){
//     int n = 100;
//     vector<int>array;
//
//
//     for( int i =0 ; i <n ;i++) array.push_back(getRandomNumber() % 100);
//
//     for( int i =0; i < n; i++) {
//         LOG("%d\n", array[i]);
//     }
//     LOG("Sorting...\n");
//     obliviousSort(array);
//     for( int i =0; i < n; i++) {
//         LOG("%d\n", array[i]);
//     }
//
// }

void kmeans(int pBlockSize, int mEnablePadding, int mSortType, string pCentroidFile, string pCoordinateFile, string pOutputPath) {
    // test_sort();
    // return;
    // // kmeans_encoder();
    // test_point();
    // return;
    bool enablePadding = mEnablePadding >0? true : false;
    int xTimes = (pBlockSize + 2048 - 1) / 2048;
    const int BlockSize = pBlockSize;//2048 * xTimes;
    const int RecordLimit = 30 * xTimes;
    // LOG("Block Size: %d\n", BlockSize);
    // LOG("Record Limit: %d\n", RecordLimit);

    string inputName;
    inputName = pCoordinateFile;//"../data/input/coordinates.txt";
    vector<string>inFiles{inputName};
    string outPath = pOutputPath;//"../data/output/";
    string centroidFile = pCentroidFile;//"../data/input/centroids.txt";
    //centroid file
    BlockFile CBF(centroidFile, IOMode_READ, BlockSize, RecordLimit, true);
    Job<StringWritable,Point,StringWritable,Point,StringWritable,Point> job;
    KMeansMapper* kMeansMapper = new KMeansMapper();
    kMeansMapper->bf = &CBF;
    kMeansMapper->mBlockSize = BlockSize;

    job.setInputFiles(inFiles);
    job.setOutputPath(outPath);
    job.setMapper(kMeansMapper);
    job.setReducer(new KmeansReducer());
    job.setBlockSize(BlockSize);
    job.setBlockRecordLimit(RecordLimit);
    job.run(enablePadding, mSortType);
}
