
#ifndef __WRITABLE__
#define __WRITABLE__

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <utility>
#include "utils.h"
#include <string>
using namespace std;

class Writable {
public:
    virtual int write(char* buf) = 0;
    virtual int read(char* buf) = 0;
};

class IntWritable : public Writable {
private:
    const int size = 4; // bytes
    int value;
public:
    bool operator <(const IntWritable& obj) const;
    IntWritable& operator=(IntWritable obj);
    IntWritable();
    IntWritable(int pValue);
    void setValue(int pValue);
    int getValue()const;
    int getSize();
    void getBytes(char* bytes);
    int read(char* buf);
    int write(char* buf);
};

class StringWritable : public Writable {
private:
    IntWritable size;
    string value;    
public:
    bool operator <(const StringWritable& obj) const;
    bool operator !=(const StringWritable& obj) const;
    StringWritable& operator=(StringWritable pObj);
    StringWritable();
    StringWritable(string pValue);
    void setValue(string pValue);
    string getValue()const;
    int getSize();
    int read(char* buf);
    int write(char* buf);
};


class DBKey : Writable {
public:
    StringWritable key;
    StringWritable tableName;
    bool operator <(const DBKey& pObj) const;
    DBKey& operator=(DBKey pObj);
    DBKey();
    DBKey(StringWritable pKey, StringWritable pTableName);
    void setKey(StringWritable pKey);
    void setTableName(StringWritable pTableName);
    StringWritable getKey();
    StringWritable getTableName();
    int getSize();
    int read(char* buf);
    int write(char* buf);
};

class DBValue : Writable {
public:
    StringWritable first;
    StringWritable second;
    bool operator <(const DBValue& pObj) const;
    DBValue& operator=(DBValue pObj);
    DBValue();
    DBValue(StringWritable pFirst, StringWritable pSecond);
    void setFirst(StringWritable pFirst);
    void setSecond(StringWritable pSecond);
    StringWritable getFirst();
    StringWritable getSecond();
    int getSize();
    int read(char* buf);
    int write(char* buf);
};


class Point : public Writable {
public:
    vector<double>coordinates;
    int nop;// number of point combined

    Point();
    bool operator <(const Point& pObj)const;
    bool operator !=(const Point& pObj)const;
    Point& operator=(Point pObj);
    void add(Point& p);
    void merge();
    void setValue(Point pValue);
    Point getValue()const;
    int getSize();
    void decode(string  str, Point& pt);
    string encode(Point& pt);
    int read(char* bytes);
    int write(char* bytes);
};

template < class K, class V>
class Record : Writable {
public:
    K key;
    V value;
    bool operator <(const Record<K,V>& pObj) const;
    Record<K,V>& operator=(Record<K,V> pObj);
    Record();
    Record(K pKey, V pValue);
    void setKey(K pKey);
    void setValue(V pValue);
    K getKey();
    V getValue();
    int getSize();
    int read(char* buf);
    int write(char* buf);
};

#endif
