#include <string>

#include "data_type.h"

class InputWay {
public:
    InputWay(const char* file_name, data_type_t* buf, size_t buf_size);

    ~InputWay();

    InputWay(const InputWay& other) = delete;

    InputWay& operator=(const InputWay& other) = delete;

    // returns true of all data was read from the disk
    bool empty();

    // returns the next value to be read
    data_type_t top();

    // reads the next value from file (using buffer) and returns it
    data_type_t get();

private:
    int fd;
    data_type_t* buf;
    size_t pos;
    size_t size;
    size_t remain;
    std::string filename;

    // loads next large of sorted data from the tmp file
    void load_chunk();
};
