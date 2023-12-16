#include "Writable.h"


// ------------- IntWritable -------
IntWritable::IntWritable(){}
IntWritable::IntWritable(int pValue) {
    value = pValue;
}

void IntWritable::setValue(int pValue) {
    
    value = pValue;
}

int IntWritable::getValue() const { 
    return value;
}

bool IntWritable::operator <(const IntWritable& obj) const {
    return value < obj.getValue();
}

IntWritable& IntWritable::operator=(IntWritable pObj) {
    setValue(pObj.getValue());
    return *this;
}

int IntWritable::getSize() {
    return size;
}

void IntWritable::getBytes(char* bytes) {
    for (int i = 0; i < size; i++) {
        bytes[i] = 0xFF&(value >> (i * 8));
    }
}

int IntWritable::read(char* bytes) {

    int B0 = 0xFF & (bytes[0]);
    int B1 = 0xFFFF & (bytes[1] << 8);
    int B2 = 0xFFFFFF & (bytes[2] << 16);
    int B3 = 0xFFFFFFFF & (bytes[3] << 24);
    value =  0xFFFFFFFF & (B0 | B1 | B2 | B3);
    //LOG("In IntWritable: %d\n", value);
    return size; //bytesRead
}

int IntWritable::write(char* bytes) {
    for (int i = 0; i < size; i++) {
        bytes[i] = 0xFF&(value >> (i * 8));
    }
    return size; // bytesWritten
}


// ------------- StringWritable -------

StringWritable::StringWritable() {
    string v;
    v.push_back(0xFF); v.push_back(0xFF); v.push_back(0xFF);
    size.setValue(v.length());
    value = v;
}
StringWritable::StringWritable(string pValue) {
    size.setValue(pValue.length());
    value = pValue;
}

void StringWritable::setValue(string pValue) {
    size.setValue(pValue.length());
    value = pValue;
}

string StringWritable::getValue() const {
    return value;
}

int StringWritable::getSize() {
    return size.getValue() + size.getSize();
}

bool StringWritable::operator <(const StringWritable& pObj)const {
     int res = value.compare(pObj.getValue());
     return res < 0;
}

bool StringWritable::operator !=(const StringWritable& pObj)const {
     int res = value.compare(pObj.getValue());
     return res!=0;
}

StringWritable& StringWritable::operator=(StringWritable pObj) {
    setValue(pObj.getValue());
    return *this;
}

int StringWritable::read(char* bytes) {
    int iter = 0;
    
    iter += size.read(bytes);
    
    //The error is occuring right here
   // LOG("signed size.getValue(): %d\n", size.getValue()); 
    value =  string(bytes + iter, size.getValue());
    //LOG("value: %s\n", value.c_str());
    
    return iter + size.getValue();
}

int StringWritable::write(char* bytes) {
    int iter = 0;
    iter+=size.write(bytes);
    memcpy(bytes + iter, value.data(), size.getValue());
    return iter+size.getValue();
}

// -------- DB Key Implementation
bool DBKey::operator <(const DBKey& pObj) const{

    if((key < pObj.key) == (pObj.key < key)){
        return tableName < pObj.tableName;
    } else{
        return key < pObj.key;
    }

}

DBKey& DBKey::operator=(DBKey pObj) {
    setKey(pObj.getKey());
    setTableName(pObj.getTableName());
    return *this;
}

DBKey::DBKey() {}

DBKey::DBKey(StringWritable pKey, StringWritable pTableName){
    key.setValue(pKey.getValue());
    tableName.setValue(pTableName.getValue());
}

void DBKey::setKey(StringWritable pKey){
    key.setValue(pKey.getValue());
}

void DBKey::setTableName(StringWritable pValue) {
    tableName.setValue(pValue.getValue());
}

StringWritable DBKey::getKey() {
    return key;
}

StringWritable DBKey::getTableName() {
    return tableName;
}

int DBKey::getSize() {
    return key.getSize() + tableName.getSize() + 4; // TODO: to be changed with size of 1 IntWritable
}

int DBKey::read(char* buf) {
    IntWritable dbKeySize;
    int iter = 0;
    iter+=dbKeySize.read(buf);
    iter+=key.read(buf+iter);
    iter+=tableName.read(buf+iter);
    return iter;
}

int DBKey::write(char* buf){
    IntWritable dbKeySize(key.getSize() + tableName.getSize());
    // LOG("%d\n", recordSize.getValue());
    int iter = 0;
    iter+=dbKeySize.write(buf);
    iter+=key.write(buf+iter);
    iter+=tableName.write(buf+iter);
    return iter;
}

// -------- DB Value Implementation
bool DBValue::operator <(const DBValue& pObj) const{

    if((first < pObj.first) == (pObj.first < first)){
        return second < pObj.second;
    } else{
        return first < pObj.first;
    }

}

DBValue& DBValue::operator=(DBValue pObj) {
    setFirst(pObj.getFirst());
    setSecond(pObj.getSecond());
    return *this;
}

DBValue::DBValue() {}

DBValue::DBValue(StringWritable pFirst, StringWritable pSecond){
    first.setValue(pFirst.getValue());
    second.setValue(pSecond.getValue());
}

void DBValue::setFirst(StringWritable pFirst){
    first.setValue(pFirst.getValue());
}

void DBValue::setSecond(StringWritable pSecond) {
    second.setValue(pSecond.getValue());
}

StringWritable DBValue::getFirst() {
    return first;
}

StringWritable DBValue::getSecond() {
    return second;
}

int DBValue::getSize() {
    return first.getSize() + second.getSize() + 4; // TODO: to be changed with size of 1 IntWritable
}

int DBValue::read(char* buf) {
    IntWritable dbValueSize;
    int iter = 0;
    iter+=dbValueSize.read(buf);
    iter+=first.read(buf+iter);
    iter+=second.read(buf+iter);
    return iter;
}

int DBValue::write(char* buf){
    IntWritable dbValueSize(first.getSize() + second.getSize());
    // LOG("%d\n", recordSize.getValue());
    int iter = 0;
    iter+=dbValueSize.write(buf);
    iter+=first.write(buf+iter);
    iter+=second.write(buf+iter);
    return iter;
}

// --------- Point Implementation -----


    Point::Point() {
        // coordinates.push_back(0);
        // coordinates.push_back(0);
        // string v;
        // v.push_back(0xFF); v.push_back(0xFF); v.push_back(0xFF);
        // size.setValue(v.length());
        // value = v;
        nop = 0;
    }

    // Point(string pValue) {
    //     size.setValue(pValue.length());
    //     value = pValue;
    // }


    void Point::setValue(Point pt) {
        nop = pt.nop;
        coordinates.clear();
        for (size_t i =0; i <pt.coordinates.size(); i++ ) {
            coordinates.push_back(pt.coordinates[i]);
        }
    }

    Point Point::getValue() const {
        return *this;
    }

    int Point::getSize() {
        return encode(*this).size();
    }

    bool Point::operator <(const Point& pObj)const {

         if (nop == 0) return false;
         if (pObj.nop == 0) return true;
         // comparing SQ distance
         size_t sz = coordinates.size();
         for( size_t i = 0; i < sz; i++) {
            if ( coordinates[i] != pObj.coordinates[i]) {
                return coordinates[i] < pObj.coordinates[i];
            }
         }
         return true;
    }

    bool Point::operator !=(const Point& pObj)const {
        if (nop == 0) return true;
        if (pObj.nop == 0) return true;
        // comparing SQ distance
        size_t sz = coordinates.size();
        for( size_t i = 0; i < sz; i++) {
           if ( coordinates[i] != pObj.coordinates[i]) {
               return true;
           }
        }
        return false;
    }

    Point& Point::operator=(Point pObj) {
        nop = pObj.nop;
        coordinates.clear();
        for (size_t i =0; i <pObj.coordinates.size(); i++ ) {
            coordinates.push_back(pObj.coordinates[i]);
        }
        return *this;
    }

    void Point::add(Point& p) {
        size_t sz = coordinates.size();
        for( size_t i = 0; i < sz; i++) {
            coordinates[i]+=p.coordinates[i];
        }
        nop+= p.nop;
    }

    void Point::merge() {
        size_t sz = coordinates.size();
        for( size_t i = 0; i < sz; i++) {
            coordinates[i]/=nop;
        }
        nop = 1;
    }

    //decode comma seperated string to point
    //nop,val,val,val
    void Point::decode(string  str, Point& pt) {
        size_t sz = str.size();
        string strValue;
        pt.nop = -1;
        size_t i = 0;
        while( i < sz && str[i]!=',') {
            strValue.push_back(str[i++]);
        }

        pt.nop = stoi(strValue);
        strValue.clear();

        for( ++i; i < sz; i++ ) {
            if (str[i] == ',') {
                pt.coordinates.push_back(stod(strValue));
                strValue.clear();
            } else {
                strValue.push_back(str[i]);
            }
        }
        if(strValue.size()>0) {
            pt.coordinates.push_back(stod(strValue));
        }
    }

    string Point::encode(Point& pt) {
        size_t sz = pt.coordinates.size();
        string str;
        str.append(to_string(nop));
        for(size_t i = 0; i < sz; i++) {
            string strValue = to_string(coordinates[i]);
            str.push_back(',');
            str.append(strValue);
        }
        return str;
    }

    int Point::read(char* bytes) {
        coordinates.clear();
        IntWritable size;
        string value;
        int iter = 0;
        iter += size.read(bytes);
        value =  string(bytes + iter, size.getValue());
        decode(value, *this);
        return iter + size.getValue();
    }

    int Point::write(char* bytes) {
        IntWritable size;
        string value = encode(*this);
        size.setValue(value.size());
        int iter = 0;
        iter+=size.write(bytes);
        memcpy(bytes + iter, value.data(), size.getValue());
        return iter+size.getValue();
    }
// end Point Class
//------- Record Implementation
template<class K, class V>
bool Record<K,V>::operator <(const Record<K,V>& pObj) const{
    return key < pObj.key;
}

template<class K, class V>
Record<K,V>& Record<K,V>::operator=(Record<K,V> pObj) {
    setKey(pObj.getKey());
    setValue(pObj.getValue());
    return *this;
}

template<class K, class V>
Record<K,V>::Record() {}

template<class K, class V>
Record<K,V>::Record(K pKey, V pValue){
    key.setValue(pKey.getValue());
    value.setValue(pValue.getValue());
}

template<class K, class V>
void Record<K,V>::setKey(K pKey){
    key.setValue(pKey.getValue());
}

template<class K, class V>
void Record<K,V>::setValue(V pValue) {
    value.setValue(pValue.getValue());
}

template<class K, class V>
K Record<K,V>::getKey() {
    return key;
}

template<class K, class V>
V Record<K,V>::getValue() {
    return value;
}

template<class K, class V>
int Record<K,V>::getSize() {
    return key.getSize() + value.getSize() + 4; // TODO: to be changed with size of 1 IntWritable
}

template<class K, class V>
int Record<K,V>::read(char* buf) {
    IntWritable recordSize;
    int iter = 0;
    iter+=recordSize.read(buf);
    
    
   // LOG("BEFOREKEY: %s\n",key.getValue().getValue().c_str());  
          
     
    iter+=key.read(buf+iter);
    //LOG("AFTERKEY: %s\n",key);  
    
    iter+=value.read(buf+iter);
    return iter;
}

template<class K, class V>
int Record<K,V>::write(char* buf){
    IntWritable recordSize(key.getSize() + value.getSize());
    // LOG("%d\n", recordSize.getValue());
    int iter = 0;
    iter+=recordSize.write(buf);
    iter+=key.write(buf+iter);
    iter+=value.write(buf+iter);
    return iter;
}

template class Record<StringWritable, StringWritable>;
template class Record<Record<StringWritable,StringWritable>,Record<StringWritable,StringWritable>>;

template class Record<StringWritable, Point>;
 // template class Record<IntWritable, StringWritable>;
// template class Record<StringWritable,IntWritable>;
