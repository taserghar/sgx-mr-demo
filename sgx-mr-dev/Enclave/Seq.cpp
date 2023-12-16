#include "Seq.h"
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
using namespace std;

void Seq::serialize(stream_t &stream) {
    //format : len-buf-len-buf
    int pos = 0;
    int len_seq_id = seq_id.length();
    int len_seq = seq.length();

    stream.buf = new BYTE [len_seq_id + 1 + len_seq + 1];

    stream.buf[pos] = (BYTE) len_seq_id;
    pos+=1;
    memcpy(stream.buf + pos, seq_id.c_str(), len_seq_id);
    pos+=len_seq_id;

    stream.buf[pos] = (BYTE) len_seq;
    pos+=1;
    memcpy(stream.buf + pos, seq.c_str(), len_seq);
    pos+=len_seq;

    stream.len = pos;
}

void Seq::deserialize(stream_t &stream) {
    //format : len-buf-len-buf
    int pos = 0;
    int len_seq_id = (int) stream.buf[pos];
    pos+=1;
    char* seq_id_loc = new char[len_seq_id + 1]; //TODO: need to generalize
    memcpy(seq_id_loc, stream.buf + pos, len_seq_id);
    seq_id_loc[len_seq_id] = NULL;
    pos+=len_seq_id;

    int len_seq = stream.buf[pos];
    pos+=1;
    char* seq_loc = new char[len_seq + 1];
    memcpy(seq_loc, stream.buf + pos, len_seq);
    seq_loc[len_seq] = NULL;
    seq_id = seq_id_loc;
    seq = seq_loc;
}
