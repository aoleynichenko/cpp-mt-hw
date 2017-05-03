#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "InputWay.h"

using std::string;

InputWay::InputWay(const char* file_name, data_type_t* buffer, size_t buf_size) {
    struct stat st;

    buf = buffer;
    size = buf_size;
    pos = size;

    // determine, how many numbers we should read
    if (stat(file_name, &st) == 0) {
        remain = st.st_size / sizeof(data_type_t);
    } else {
        fprintf(stderr, "Error in InputWay::InputWay() stat()\n");
        return;
    }

    fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error in InputWay::InputWay() open()\n");
        return;
    }

    filename = string(file_name);
}

InputWay::~InputWay() {
    close(fd);
    unlink(filename.c_str());
}

// returns true of all data was read from the disk
bool InputWay::empty() {
    return (pos == size) && (remain == 0);
}

// returns the next value to be read
data_type_t InputWay::top() {
    if (empty()) {
        return 0;
    }
    if (pos == size) {
        load_chunk();
    }
    return buf[pos];
}

// reads the next value from file (using buffer) and returns it
data_type_t InputWay::get() {
    data_type_t top_number = top();
    pos++;
    return top_number;
}

//------------------------------
// private methods
//------------------------------

void InputWay::load_chunk() {
    if (remain == 0) { // nothing to read
        return;
    }

    // read next chunk from file to the internal buffer
    size = (remain > size) ? size : remain;
    remain -= size;
    read(fd, buf, size * sizeof(data_type_t));
    pos = 0;
}
