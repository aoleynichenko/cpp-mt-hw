// OutputWay.h
// Implementation: OutputWay.cpp
//
// A very simple output buffer.
// Here: "Way" == "Buffer" (k-way sorting)

#include "data_type.h"

#include <string>

class OutputWay {
public:
    // output way shold be constructed only for allocated buffer of size buf_size
    // NOTE: buffer size units are not [bytes]! buf should be able to contain
    // buf_size numbers of type data_type_t
    OutputWay(data_type_t* buf, size_t buf_size);

    // copy and assignment constructors are disabled because OutputWay owns its
    // single copy of file descriptor
    OutputWay(const OutputWay&) = delete;
    OutputWay& operator=(const OutputWay&) = delete;

    ~OutputWay();

    // puts 1 object of type data_type_t to the output
    // NOTE: data will be written to disk only when the buffer is full
    void put(data_type_t data);

    // constructor creates output stream file name randomly, so if the user wants
    // to use it, he should invoke this method
    std::string get_file_name();

private:
    // output file descriptor
    int fd;

    // buffer
    data_type_t* buf;

    // position in the buffer
    size_t pos;

    // buffer size
    size_t size;

    // name of the output file
    std::string file_name;
};
