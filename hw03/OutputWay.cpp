#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "OutputWay.h"

using std::string;

OutputWay::OutputWay(data_type_t* buffer, size_t buf_size) {
    buf = buffer;
    size = buf_size;
    pos = 0;

    char fname[] = "/tmp/extsortXXXXXX";

    fd = mkstemp(fname);
    if (fd == -1) {
        fprintf(stderr, "Error in OutputWay::OutputWay() open()\n");
        return;
    }

    file_name = string(fname);
}

OutputWay::~OutputWay() {
    // flush the remainder from the buffer to disk
    write(fd, buf, pos * sizeof(data_type_t));

    close(fd);
}

void OutputWay::put(data_type_t data) {
    buf[pos] = data;
    pos++;

    // flush chunk
    if (pos == size) {
        write(fd, buf, size * sizeof(data_type_t));
        pos = 0;
    }
}

string OutputWay::get_file_name() {
    return file_name;
}
