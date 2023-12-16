#pragma once


typedef unsigned char BYTE;

struct stream_t {
    BYTE* buf;
    int len;
};

class Serializable {
public:
    Serializable(){}
    virtual ~Serializable(){}

    virtual void serialize(stream_t &stream) = 0;
    virtual void deserialize(stream_t &stream) = 0;
};
