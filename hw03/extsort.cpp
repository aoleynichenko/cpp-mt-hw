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

struct InputWay {  // "sorting way"
    int fd;
    char filename[16];
    data_type* buf;
    size_t pos;
    size_t size;
    size_t remain;

    InputWay() {  // dummy for new
    }

    InputWay(data_type* data, size_t nelem, size_t way_size)
      :   remain(nelem), buf(nullptr), size(way_size) {
        pos = size;
        strcpy(filename, "/tmp/extsortXXXXXX");
        strncpy(filename, tmpnam(NULL), 16);
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        //fd = mkstemp(filename);  // O_RDWR
        //printf("fd = %d\n", fd);
        // flush all data of this way to file
        write(fd, data, nelem * sizeof(data_type));
        close(fd);
        /*fd = open(filename, O_RDONLY);
        data_type dt[100];
        for (int i = 0; i < nelem; i++) {
          read(fd, dt, 4);
          printf(">> %u\n", dt[0]);
        }
        printf("fd = %d\n", fd);*/
        //lseek(fd, 0, SEEK_SET);   // we start reading from the beginning
    }

    ~InputWay() {
        if (buf != nullptr) {
            delete[] buf;
        }
        //close(fd);
        unlink(filename);
    }

    bool empty() {
        return (pos == size) && (remain == 0);
    }

    data_type curr_data() {
        if (empty()) {
            //printf("empty\n");
            //print();
            return 0;
        }
        if (pos == size) {
            load_next();
        }
        return buf[pos];
    }

    data_type read_data() {
        if (empty()) {
            printf("is empty\n");
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
            printf("nothing to read\n");
            return;
        }
        size = (remain > size) ? size : remain;
        remain -= size;
        printf("%d %s load next %d elements\n", this->fd, filename, size);
        printf("remain = %u\n", remain);
        fd = open(filename, O_RDONLY);
        size_t read_b = read(fd, buf, size * sizeof(data_type));
        printf("read %d bytes\n", read_b);
        if (read_b == -1)
          printf("error: %s\n", strerror(errno));
        printf("read value = %u\n", buf[0]);
        close(fd);
        pos = 0;
    }

    void print() {
        printf("filename = %s\n", filename);
        printf("pos = %u\n", pos);
        printf("size = %u\n", size);
        printf("remain = %u\n\n", remain);
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
    int memsize = mb;// * 1024 * 1024 / sizeof(data_type);
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
    printf("MEM SIZE = %d\n", memsize);
    printf("WAY SIZE = %d\n", way_size);
    printf("%d-way sorting\n", k);
    InputWay *ways = new InputWay[k];

    for (int i = 0; i < k; i++) {
        // надо посчитать, сколько байт отрезать в каждый путь
        int nelem = (totalsize < memsize) ? totalsize : memsize;
        printf("%d/%d %d bytes\n", i, k, nelem*sizeof(data_type));
        size_t rd = read(ind, numbuf, nelem*sizeof(data_type));  // use return value or not?
        //printf("read %u\n")
        //for (int j = 0; j < nelem; j++)
        //    printf("%u\n", numbuf[j]);
        qsort(numbuf, nelem, sizeof(data_type), compare);

        ways[i] = InputWay(numbuf, nelem, way_size);

        totalsize -= memsize;
    }
    delete[] numbuf;

    int outd = open(out_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    printf("outd = %d\n", outd);
    size_t j = 0;
    data_type* outbuf = new data_type[way_size];
    while (true) {
        // find max element
        data_type max = 0;
        InputWay *max_way = nullptr;
        for (int i = 0; i < k; i++) {
            data_type xi = ways[i].curr_data();
            //printf("xi = %u\n", xi);
            if (!ways[i].empty() && (xi >= max)) {
                max = xi;
                max_way = ways + i;
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
    close(ind);
    delete[] ways;
}
