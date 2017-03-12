#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    char* fname = argv[1];
    int n = atoi(argv[2]);
    int fd = open(fname, O_RDONLY);
    uint32_t num;
    for (int i =0; i < n; i++) {
        read(fd, &num, 4);
        printf("%u\n", num);
    }
    close(fd);
}
