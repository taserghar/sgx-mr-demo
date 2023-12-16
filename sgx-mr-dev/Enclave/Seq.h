#include "Serializable.h"
#include <string>
class Seq: public Serializable {
public:
    std::string seq_id;
    std::string seq;
    void print();
    void serialize(stream_t &stream);
    void deserialize(stream_t &stream);
};
