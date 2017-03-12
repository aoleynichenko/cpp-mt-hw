// internal sorting using qsort

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "data_type.h"

void usage()
{
    printf("Usage: intsort <inp-file> <out-file>\n");
    printf(" <inp-file>  file with unsorted data\n");
}

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

int main(int argc, char **argv)
{
    int ind, outd;
    char *in_name, *out_name;
    data_type *buf;
    size_t bufsize, n_read;
    off_t fsz;  // size (bytes) of the input file

    if (argc != 3) {
        usage();
        return 1;
    }
    in_name = argv[1];
    out_name = argv[2];

    fsz = fsize(in_name);
    if (fsz == -1) {
        fprintf(stderr, "Error: cannot determine size of %s: %s\n",
            in_name, strerror(errno));
        return 2;
    }
    bufsize = fsz / sizeof(data_type);
    buf = (data_type *) malloc (bufsize * sizeof(data_type));

    // read numbers
    ind = open(in_name, O_RDONLY);
    n_read = read(ind, buf, sizeof(data_type) * bufsize);
    if (n_read != fsz) {
        fprintf(stderr, "Error: not all data from file were read\n");
        free(buf);
        close(ind);
        return 2;
    }
    close(ind);

    // sort number in buffer
    qsort(buf, bufsize, sizeof(data_type), data_less);

    // write results to the output file
    outd = open(out_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    write(outd, buf, bufsize * sizeof(data_type));
    close(outd);

    free(buf);
    return 0;
}
