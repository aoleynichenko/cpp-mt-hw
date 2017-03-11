// External sorting code
// Homework 3
// Alexander Oleynichenko, 2017

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef uint32_t data_type;

void gen_file(int mb)
{
    srand(time(NULL));

    uint64_t n_entries = mb * 1024 * 1024 / sizeof(data_type);
    FILE* to = fopen("uint32_t.data", "rw");

    for (uint64_t i = 0; i < n_entries; i++)
        fprintf(to, "%u", rand());
    fclose(to);
}

int main(int argc, char** argv)
{
    gen_file(3);
}
