// Homework 3
// External sorting code
// Alexander Oleynichenko, 2017
// mailto: ao2310@yandex.ru

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "data_type.h"

class InputWay {  // "sorting way"
private:
    int fd;
    char filename[32];
    data_type* buf;
    size_t pos;
    size_t size;
    size_t remain;
public:
    InputWay(data_type* data, size_t nelem, size_t way_size)
      :   remain(nelem), buf(nullptr), size(way_size) {
        pos = size;
        strcpy(filename, "/tmp/extsortXXXXXX");
        fd = mkstemp(filename);  // O_RDWR
        write(fd, data, nelem * sizeof(data_type));
        lseek(fd, 0, SEEK_SET);   // we start reading from the beginning
    }

    ~InputWay() {
        if (buf != nullptr) {
            delete[] buf;
        }
        close(fd);
        unlink(filename);
    }

    InputWay(const InputWay& other) = delete;

    InputWay& operator=(const InputWay& other) = delete;

    bool empty() {
        return (pos == size) && (remain == 0);
    }

    data_type curr_data() {
        if (empty()) {
            return 0;
        }
        if (pos == size) {
            load_next();
        }
        return buf[pos];
    }

    data_type read_data() {
        if (empty()) {
            return 0;
        }
        if (pos == size) {
            load_next();
        }
        pos++;
        return buf[pos-1];
    }

    void load_next() {
        if (buf == nullptr) {
            buf = new data_type[size];
        }
        if (remain == 0) {  // nothing to read
            return;
        }
        size = (remain > size) ? size : remain;
        remain -= size;
        size_t read_b = read(fd, buf, size * sizeof(data_type));
        pos = 0;
    }
};

void usage() {
    printf("Usage: extsort <inp-file> <out-file> <mb>\n");
    printf(" <inp-file>  path to file with unsorted data\n");
    printf(" <out-file>  path to file to which sorted data will be written\n");
    printf(" <mb>        size of memory to be used for sorting (megabytes)\n");
}

// returns number of data_type entries in file
size_t file_size(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size / sizeof(data_type);

    return -1;
}

int str_to_positive(char *s) {
    char *end;
    int n = strtol(s, &end, 10);
    if (n <= 0)
        return -1;
}

#include <vector>
using std::vector;

int main(int argc, char **argv)
{
    int mb;    // n(megabytes of RAM) to be used
    int ind;   // input file descriptor
    size_t n;  // n(entries)
    size_t k;  // K-way sorting
    char *inp_name, *out_name;
    data_type *numbuf;

    if (argc != 4) {
        usage();
        return 1;
    }

    inp_name = argv[1];
    out_name = argv[2];
    mb = str_to_positive(argv[3]);
    if (mb < 0) {
        fprintf(stderr, "Error: wrong number of megabytes\n");
        return 1;
    }

    n = file_size(inp_name);
    if (n == -1) {
        fprintf(stderr, "Error: input file '%s' not found\n", inp_name);
        return 1;
    }

    // alloc buffer for numbers from input file
    int memsize = mb * 1024 * 1024 / sizeof(data_type);
    numbuf = new data_type[memsize];
    if (numbuf == NULL) {
        fprintf(stderr, "Error: unable to allocate %d bytes\n", n * sizeof(data_type));
        return 1;
    }
    ind = open(inp_name, O_RDONLY);

    // split large unsorted file into k sorted files
    // here I use qsort() for simplicity
    int totalsize = n;
    k = (size_t) ceil((double) n / memsize);
    int way_size = memsize / (k+1);
    printf("%d-way sorting\n", k);
    //InputWay *ways = new InputWay[k];
    vector<InputWay*> ways;

    for (int i = 0; i < k; i++) {
        // надо посчитать, сколько байт отрезать в каждый путь
        int nelem = (totalsize < memsize) ? totalsize : memsize;
        printf("%d/%d %d bytes\n", i, k, nelem*sizeof(data_type));
        size_t rd = read(ind, numbuf, nelem*sizeof(data_type));  // use return value or not?

        qsort(numbuf, nelem, sizeof(data_type), compare);

        ways.push_back(new InputWay(numbuf, nelem, way_size));

        totalsize -= memsize;
    }
    delete[] numbuf;
    close(ind);

    int outd = open(out_name, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    //printf("outd = %d\n", outd);
    size_t j = 0;
    data_type* outbuf = new data_type[way_size];
    //printf("Start loop\n");
    while (true) {
        // find max element
        data_type max = 0;
        InputWay *max_way = nullptr;
        for (int i = 0; i < k; i++) {
            data_type xi = ways[i]->curr_data();
            //printf("xi = %u\n", xi);
            if (!ways[i]->empty() && (xi >= max)) {
                max = xi;
                max_way = ways[i];
            }
        }
        if (max_way == nullptr) {  // finished
            break;
        }
        data_type x = max_way->read_data();
        //printf("max = %u\n", x);
        outbuf[j] = x;
        j++;
        if (j == way_size) {
            write(outd, outbuf, way_size*sizeof(data_type));
            j = 0;
        }
    }
    write(outd, outbuf, j*sizeof(data_type));
    delete[] outbuf;

    close(outd);
}
