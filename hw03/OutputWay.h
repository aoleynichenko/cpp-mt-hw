#include "data_type.h"

#include <string>

class OutputWay {
public:
    OutputWay(data_type_t* buf, size_t buf_size);

    OutputWay(const OutputWay&) = delete;

    OutputWay& operator=(const OutputWay&) = delete;

    ~OutputWay();

    void put(data_type_t data);

    std::string get_file_name();

private:
    int fd;
    data_type_t* buf;
    size_t pos;
    size_t size;
    std::string file_name;
};
