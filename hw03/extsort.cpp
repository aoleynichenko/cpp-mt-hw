// Homework 3
// External sorting code (k-way 1-pass external merge sort)
// Alexander Oleynichenko, 2017
// mailto: ao2310@yandex.ru
//
// Algorithm details:
// https://en.wikipedia.org/wiki/External_sorting#External_merge_sort

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <vector>

using std::vector;

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
    InputWay(data_type* data, size_t nelem, size_t buf_size)
      :   remain(nelem), buf(nullptr), size(buf_size) {
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

    // returns true of all data was read from the disk
    bool empty() {
        return (pos == size) && (remain == 0);
    }

    // returns the next value to be read
    data_type* curr_data() {
        if (empty()) {
            return 0;
        }
        if (pos == size) {
            load_next();
        }
        return &buf[pos];
    }

    // reads the next value from file (using buffer) and returns it
    data_type* read_data() {
        if (empty()) {
            return 0;
        }
        if (pos == size) {
            load_next();
        }
        pos++;
        return &buf[pos-1];
    }

    // loads next large of sorted data from the tmp file
    void load_next() {
        if (buf == nullptr) {
            buf = new data_type[size];
        }
        if (remain == 0) {  // nothing to read
            return;
        }
        size = (remain > size) ? size : remain;
        remain -= size;
        read(fd, buf, size * sizeof(data_type));
        pos = 0;
    }
};

class OutputWay {
private:
    int fd;
    data_type* buf;
    size_t pos;
    size_t size;
public:
    OutputWay(char* filename, size_t buf_size)
      : pos(0), size(buf_size)
    {
        buf = new data_type[buf_size];
        fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    }

    OutputWay(const OutputWay&) = delete;

    OutputWay& operator=(const OutputWay&) = delete;

    ~OutputWay() {
        // flush the remainder in the buffer
        write(fd, buf, pos * sizeof(data_type));

        delete[] buf;
        close(fd);
    }

    void put(data_type* data) {
        buf[pos] = *data;
        pos++;
        if (pos == size) {
            write(fd, buf, size * sizeof(data_type));
            pos = 0;
        }
    }
};

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

void usage() {
    printf("Usage: extsort <inp-file> <out-file> <mb>\n");
    printf(" <inp-file>  path to file with unsorted data\n");
    printf(" <out-file>  path to file to which sorted data will be written\n");
    printf(" <mb>        size of memory to be used for sorting (megabytes)\n");
}

int main(int argc, char **argv)
{
    int mb;    // n(megabytes of RAM) to be used
    char *inp_name, *out_name;
    clock_t t0, t1, t2;

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

    size_t n = file_size(inp_name);  // n(entries)
    if (n == -1) {
        fprintf(stderr, "Error: input file '%s' not found\n", inp_name);
        return 1;
    }

    // alloc buffer for numbers from input file
    size_t memsize = (size_t) mb * 1024 * 1024 / sizeof(data_type);
    data_type* numbuf = new data_type[memsize];
    if (numbuf == NULL) {
        fprintf(stderr, "Error: unable to allocate %d bytes\n", n * sizeof(data_type));
        return 1;
    }

    // split large unsorted file into k sorted files
    // here I use qsort() for simplicity
    size_t totalsize = n;
    size_t k = (size_t) ceil((double) n / memsize);   // K-way sorting
    size_t way_size = memsize / (k+1);
    t0 = clock();
    printf("%d-way sorting\n", k);

    vector<InputWay*> ways;
    int ind = open(inp_name, O_RDONLY);  // read large input file chunk by chunk
    for (size_t i = 0; i < k; i++) {
        size_t nelem = (totalsize < memsize) ? totalsize : memsize;
        totalsize -= memsize;

        read(ind, numbuf, nelem*sizeof(data_type));
        qsort(numbuf, nelem, sizeof(data_type), data_less);
        ways.push_back(new InputWay(numbuf, nelem, way_size));

        printf("%d/%d %llu bytes\n", i+1, k, nelem*sizeof(data_type));
    }
    t1 = clock();
    delete[] numbuf;
    close(ind);

    // merge sorted files
    OutputWay out(out_name, way_size);
    while (true) {
        // find max element
        data_type start_value = DATA_MIN_VALUE;
        data_type* max = &start_value;
        InputWay* max_way = nullptr;

        for (size_t i = 0; i < k; i++) {
            data_type* xi = ways[i]->curr_data();
            if (!ways[i]->empty() && data_less(max, xi)) {
                max = xi;
                max_way = ways[i];
            }
        }
        if (max_way == nullptr) {  // finished
            break;
        }

        out.put(max_way->read_data());
    }
    t2 = clock();

    // final cleanup
    // close and remove all temporary files, free buffers
    for (size_t i = 0; i < k; i++)
        delete ways[i];

    // statistics
    printf("----------------------------\n");
    printf("Time for splitting and qsort  %.2f sec\n", (double)(t1 - t0)/CLOCKS_PER_SEC);
    printf("Time for %d-way merge         %.2f sec\n", k, (double)(t2 - t1)/CLOCKS_PER_SEC);
}
