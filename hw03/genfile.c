// genfile.c
// ---------
// Tiny auxiliary program designed to generate test sets for the external
// sorting code.
// Output: sorted and unsorted files with uint32_t numbers
//
// Alexander Oleynichenko, 2017
// ao2310@yandex.ru

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "data_type.h"

int data_less(const void* a, const void* b) {
    return *(data_type_t*)a < *(data_type_t*)b;
}

#define MAX_MB 1024 // 1 GB (it is just a test)
#define MAX_ELEMS (MAX_MB * 1024 * 1024 / sizeof(data_type_t))

void usage() {
    printf("Usage: genfile --sz <mb> [--sorted <sorted-name>] [--unsorted <unsorted-name>]\n");
}

void error(char* message) {
    fprintf(stderr, "Error: %s\n", message);
}

void fill_random(data_type_t* buf, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        buf[i] = rand();
    }
}

void flush_numbers(data_type_t* buf, size_t n, char* file_name) {
    size_t i;
    int fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    write(fd, buf, n * sizeof(data_type_t));
    close(fd);
}

int main(int argc, char** argv) {
    char* default_sorted_name = "sorted";
    char* default_unsorted_name = "unsorted";

    char* sorted_name = default_sorted_name;
    char* unsorted_name = default_unsorted_name;
    char** argp;
    size_t sz = 0;
    data_type_t* numbuf = NULL;
    int nosorted = 0;

    // parse command line arguments
    if (argc == 1) {
        usage();
        return 1;
    }
    argc--;
    argp = argv + 1;
    while (argc) {
        if (strcmp(*argp, "--sz") == 0) {
            if (argc >= 2) {
                char* end;
                int mb = strtol(*(argp + 1), &end, 10);
                if (mb <= 0 || errno == ERANGE) {
                    error("wrong size");
                    return 1;
                }
                // megabytes -> n_elems
                sz = (int64_t)mb * 1024 * 1024 / sizeof(data_type_t);
            } else {
                error("wrong size");
                return 1;
            }
            argp += 2;
            argc -= 2;
        } else if (strcmp(*argp, "--sorted") == 0) {
            if (argc >= 2) {
                sorted_name = *(argp + 1);
            } else {
                error("expected name of sorted file");
                return 1;
            }
            argp += 2;
            argc -= 2;
        } else if (strcmp(*argp, "--unsorted") == 0) {
            if (argc >= 2) {
                unsorted_name = *(argp + 1);
            } else {
                error("expected name of unsorted file");
                return 1;
            }
            argp += 2;
            argc -= 2;
        } else if (strcmp(*argp, "--nosorted") == 0) {
            nosorted = 1;
            argp++;
            argc--;
        } else {
            error("unknown option");
            return 1;
        }
    }
    if (sz == 0) {
        error("the size should be defined (option --sz)");
        return 1;
    }
    if (sz > MAX_ELEMS && !nosorted) {
        fprintf(stderr, "Error: required file is to lagre for sorting.\n");
        fprintf(stderr, "(%u mb, limit is %u mb). ", sz * sizeof(data_type_t) / 1024 / 1024, MAX_MB);
        fprintf(stderr, "Use option --nosorted.\n");
        fprintf(stderr, "Note: no sorted file for testing will be generated.\n");
        return 1;
    }

    if (!nosorted) {
        numbuf = (data_type_t*)malloc(sz * sizeof(data_type_t));
        if (numbuf == NULL) {
            error("unable to allocate buffer");
            return 2;
        }
        fill_random(numbuf, sz);
        flush_numbers(numbuf, sz, unsorted_name);

        // now we prepare file with sorted numbers
        // output of our extsort program wil be compared to the 'sorted' file
        qsort(numbuf, sz, sizeof(data_type_t), data_less);
        flush_numbers(numbuf, sz, sorted_name);

        free(numbuf);
    } else { // generate only unsorted file
        int out = open(unsorted_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        size_t bufsize = MAX_MB * 1024 * 1024 / sizeof(data_type_t);
        size_t n = sz; // n(elements) to write on disk
        numbuf = (data_type_t*)malloc(bufsize * sizeof(data_type_t));
        while (n > 0) {
            size_t nelem = (n > bufsize) ? bufsize : n;
            n -= nelem;
            fill_random(numbuf, nelem);
            write(out, numbuf, nelem * sizeof(data_type_t));
        }
        close(out);
        free(numbuf);
    }

    return 0;
}
