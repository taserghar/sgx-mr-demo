// AKM Mubashwir Alam
// TAIC Lab, Computer Science, Marquette University
#include "kmeans.h"
using namespace std;

class KMeansMapper : public IMapper<StringWritable,Point,StringWritable,Point> {

public:
    int mBlockSize;
    int mRecordLimit;
    vector<Record<StringWritable,Point>>centroids;
    BlockFile* bf;
    void setup(){
        char* block = new char[mBlockSize];
        for( int ind = 0; bf->readBlock(block, ind)!=0 ; ind++) {
               vector<Record<StringWritable,Point>>recordList;
                RecordInputStream<StringWritable,Point>ris(block);
                ris.readRecord(recordList);
                centroids.insert(centroids.end(), recordList.begin(), recordList.end());
        }
        delete[] block;
    }

    double findDist(Point pt1, Point pt2) {
            int dim = (int) pt1.coordinates.size();
            double sum=0;
            for (int i=0; i<dim; i++){
                double d = pt1.coordinates[i]-pt2.coordinates[i];
                sum += d*d;
            }
            return sqrt(sum);
        }


    string findClosestPoint(Point pt) {
        double min_dist =  INF;

        string ret;

        for( auto& r: centroids) {

            string key= r.getKey().getValue();
            Point center = r.getValue();
            double dist = findDist(pt, center);
            if ( dist < min_dist ) {
                min_dist = dist;
                ret = key;
            }
            
        }
        return ret;
    }
    
    void mapp(StringWritable key, Point pt){

        string offset = key.getValue();
        string closest_label = findClosestPoint(pt);
        write(closest_label, pt);
    }

    Record<StringWritable,Point> combine(StringWritable key, vector<Point>& values) {
        Point pt;
        pt.coordinates.push_back(0);
        pt.coordinates.push_back(0);
        for(auto v: values) pt.add(v);
        return Record<StringWritable,Point>(key, pt);

    }
};


class KmeansReducer:public IReducer<StringWritable,Point,StringWritable,Point> {
public:

    Record<StringWritable,Point> reduce(StringWritable key, vector<Point>& values) {
        Point pt;
        pt.coordinates.push_back(0);
        pt.coordinates.push_back(0);

        for(auto v: values) {
            pt.add(v);
        }
        return Record<StringWritable,Point>(key, pt);

    }

};