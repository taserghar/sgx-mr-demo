// AKM Mubashwir Alam
// TAIC Lab, Computer Science, Marquette University

#include "wordcount.h"
using namespace std;

class MyMapper : public IMapper<StringWritable,StringWritable,StringWritable,StringWritable> {
public:
    void mapp(StringWritable key, StringWritable value){
        string line = value.getValue();
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
};


class MyReducer:public IReducer<StringWritable,StringWritable,StringWritable,StringWritable> {
public:

    Record<StringWritable,StringWritable> reduce(StringWritable key, vector<StringWritable>& values) {
        int res = 0;
        for(auto v: values) {
            res += atoi(v.getValue().c_str());
        }
        StringWritable value(StringWritable(to_string(res)));
        return Record<StringWritable,StringWritable>(key, value);

    }
};
