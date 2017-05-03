#include <string>

#include "data_type.h"

class InputWay {
public:
    // the object of type InputWay uses external buffer 'buf' of size 'buf_size'
    InputWay(const char* file_name, data_type_t* buf, size_t buf_size);

    InputWay(const InputWay& other) = delete;

    InputWay(InputWay&& other);

    ~InputWay();

    InputWay& operator=(const InputWay& other) = delete;

    // returns true of all data was read from the disk
    bool empty();

    // returns the next value to be read
    data_type_t top();

    // reads the next value from file (using buffer) and returns it
    data_type_t get();

private:
    int fd;          // associated input file
    data_type_t* buf;
    size_t pos;      // current position in buffer (in range 0...size-1)
    size_t size;     // buf size
    size_t remain;   // total number of bytes remain to read from the input file
    std::string filename;

    // loads next large of sorted data from the tmp file
    void load_chunk();
};
