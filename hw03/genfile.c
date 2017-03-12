#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "data_type.h"

#define MAX_MB 1024  // 1 GB (it is just a test)

void usage()
{
    printf("Usage: genfile --sz <mb> [--sorted <sorted-name>] [--unsorted <unsorted-name>]\n");
}

void error(char *message)
{
    fprintf(stderr, "Error: %s\n", message);
}

void fill_random(data_type *buf, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        buf[i] = rand();
    }
}

void flush_numbers(data_type *buf, size_t n, char *file_name)
{
    size_t i;
    int fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    write(fd, buf, n * sizeof(data_type));
    close(fd);
}

int main(int argc, char **argv)
{
    char *default_sorted_name = "sorted";
    char *default_unsorted_name = "unsorted";

    char *sorted_name = default_sorted_name;
    char *unsorted_name = default_unsorted_name;
    char **argp;
    size_t bufsize = 0;
    data_type *numbuf = NULL;

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
                char *end;
                int mb = strtol(*(argp+1), &end, 10);
                if (mb <= 0 || errno == ERANGE) {
                    error("wrong size");
                    return 1;
                }
                // megabytes -> n_elems
                bufsize = mb * 1024 * 1024 / sizeof(data_type);
            }
            else {
                error("wrong size");
                return 1;
            }
            argp += 2;
            argc -= 2;
        }
        else if (strcmp(*argp, "--sorted") == 0) {
            if (argc >= 2) {
                sorted_name = *(argp + 1);
            }
            else {
                error("expected name of sorted file");
                return 1;
            }
            argp += 2;
            argc -= 2;
        }
        else if (strcmp(*argp, "--unsorted") == 0) {
            if (argc >= 2) {
                sorted_name = *(argp + 1);
            }
            else {
                error("expected name of unsorted file");
                return 1;
            }
            argp += 2;
            argc -= 2;
        }
        else {
            error("unknown option");
            return 1;
        }
    }
    if (bufsize == 0) {
        error("the size should be defined (option --sz)");
        return 1;
    }

    // generate random numbers
    numbuf = (data_type *) malloc(bufsize * sizeof(data_type));
    if (numbuf == NULL) {
        error("unable to allocate buffer");
        return 2;
    }
    fill_random(numbuf, bufsize);
    flush_numbers(numbuf, bufsize, unsorted_name);

    // now we prepare file with sorted numbers
    // output of our extsort program wil be compared to the 'sorted' file
    qsort(numbuf, bufsize, sizeof(data_type), data_less);
    flush_numbers(numbuf, bufsize, sorted_name);

    free(numbuf);

    return 0;
}
