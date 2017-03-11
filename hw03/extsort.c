// Homework 3
// External sorting code
// Alexander Oleynichenko, 2017
// mailto: ao2310@yandex.ru

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "data_type.h"

struct way {  // "sorting way"
    int fd;
    char filename[16];
    data_type *buf;
    size_t pos;
    size_t size;
    size_t total;
};

void usage()
{
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

int str_to_positive(char *s)
{
    char *end;
    int n = strtol(s, &end, 10);
    if (n <= 0)
        return -1;
}

struct way *split(data_type *numbuf, size_t k) {}

void print_way_info(struct way *w)
{
    printf("w->filename = %s\n", w->filename);
    printf("w->pos = %d\n", w->pos);
    printf("w->size = %d\n", w->size);
    printf("w->total = %d\n\n", w->total);
}

int main(int argc, char **argv)
{
    int mb;  // n(megabytes of RAM) to be used
    int ind; // input file descriptor
    size_t n;   // n(entries)
    size_t k;   // K-way sorting
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
    numbuf = (data_type *) malloc(n * sizeof(data_type));
    if (numbuf == NULL) {
        fprintf(stderr, "Error: unable to allocate %d bytes\n", n * sizeof(data_type));
        return 1;
    }
    ind = open(inp_name, O_RDONLY);

    // split large unsorted file into k sorted files
    // here I use qsort() for simplicity
    int totalsize = n;
    int memsize = mb * 1024 * 1024 / sizeof(data_type);
    k = (size_t) ceil((double) n / memsize);
    printf("%d-way sorting\n", k);
    struct way *ways = (struct way *) malloc(k * sizeof(struct way));

    for (int i = 0; i < k; i++) {
        // надо посчитать, сколько байт отрезать в каждый путь
        int nelem = (totalsize < memsize) ? totalsize : memsize;
        printf("%d/%d %d\n", i, k, nelem*sizeof(data_type));
        read(ind, numbuf, nelem);
        qsort(numbuf, nelem, sizeof(data_type), compare);
        ways[i].total = nelem;
        strcpy(ways[i].filename, "/tmp/extsortXXXXXX");
        ways[i].fd = mkstemp(ways[i].filename);
        write(ways[i].fd, numbuf, nelem * sizeof(data_type));
        lseek(ways[i].fd, 0, SEEK_SET);
        totalsize -= memsize;
    }
    close(ind);

    // весь буфер надо поделить на k+1 примерно равных блоков
    // и создать выходной файл
    struct way out;
    out.fd = open(out_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    strncpy(out.filename, out_name, 16);  // вообще не нужно

    int way_size = memsize / (k+1);
    for (int i = 0; i < k; i++) {
        ways[i].pos = 0;
        ways[i].size = way_size;
        ways[i].buf = numbuf + way_size;
        print_way_info(ways + i);
    }
    out.pos = 0;
    out.size = memsize - way_size * k;

    printf("out:\n");
    print_way_info(&out);

    // удаление временных файлов и освобождение памяти
    close(out.fd);
    for (int i = 0; i < k; i++) {
        close(ways[i].fd);
        unlink(ways[i].filename);
    }
    free(ways);
    free(numbuf);
}
