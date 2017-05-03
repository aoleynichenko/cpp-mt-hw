// Homework 3
// MANY-pass K-way external sorting code
//
// Alexander Oleynichenko, 2017
// mailto: ao2310@yandex.ru
//
// Algorithm details:
// https://en.wikipedia.org/wiki/External_sorting#External_merge_sort
// https://en.wikipedia.org/wiki/External_sorting#Additional_passes

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "InputWay.h"
#include "OutputWay.h"
#include "data_type.h"

#include <queue>
#include <string>
#include <vector>

// maximum number of input ways used to perform one merge operation
// should be less or equal to the limit of opened files per process
#define MAXWAYS 20

using std::queue;
using std::string;
using std::vector;

// comparator
int data_less(const void* a, const void* b) {
    return *(data_type_t*)a < *(data_type_t*)b;
}

int split(const string& unsorted_file, queue<string>& chunks_filenames, size_t nbytes);
int merge(queue<string>& inputs, queue<string>& outputs, data_type_t* buf, size_t buf_size);

int main(int argc, char** argv) {
    // NOTE: parsing of argv is rather inaccurate
    if (argc != 4) {
        printf("Usage: %s <inp-file> <out-file> <mb>\n", argv[0]);
        return 1;
    }

    string unsorted_filename = argv[1];
    string sorted_filename = argv[2];
    int mb = atoi(argv[3]);
    printf("Input file = '%s' Output file = '%s' MB = %d\n",
        unsorted_filename.c_str(), sorted_filename.c_str(), mb);

    queue<string> queue1, queue2;
    queue<string>* curr_queue = &queue1;
    queue<string>* next_queue = &queue2;

    // split stage
    int err = split(unsorted_filename, *curr_queue, (size_t)mb * 1024 * 1024);
    if (err == -1) {
        perror("in split() ");
        return 1;
    }

    size_t nbytes = (size_t)mb * 1024 * 1024;
    size_t buf_size = nbytes / sizeof(data_type_t);
    data_type_t* buf = new data_type_t[buf_size];

    // merge stage
    printf("merge\n");
    while (curr_queue->size() != 1) {
        printf("new level\n");
        while (!curr_queue->empty()) {
            merge(*curr_queue, *next_queue, buf, buf_size);
        }
        // swap queues
        if (curr_queue == &queue1) {
            curr_queue = &queue2;
            next_queue = &queue1;
        } else {
            curr_queue = &queue1;
            next_queue = &queue2;
        }
    }

    delete[] buf;

    // copy resulting tmp file
    char cmd_buf[1024];
    sprintf(cmd_buf, "mv %s %s", curr_queue->front().c_str(), sorted_filename.c_str());
    system(cmd_buf);
}

// this function splits large unsorted file into many sorted little files (chunks)
// Algorithm:
//  1. determine number of entries N in the large input file
//  2. allocate buffer (as long as possible) for K entries
//  3. read K elements from input file, sort them and write sorted numbers to
//     the new output file
//  4. put the name of the output file to the queue (remember it)
//  5. if input is not empty goto [3]
// Output:
//  queue with names of sorted files

int split(const string& unsorted_file, queue<string>& chunks_filenames, size_t nbytes) {
    struct stat st;
    size_t n; // total number of entries

    printf("split\n");

    // get the number of data entries in the (very large) input file
    if (stat(unsorted_file.c_str(), &st) == 0) {
        n = st.st_size / sizeof(data_type_t);
    } else {
        return -1; // file not found
    }

    // number of entries we can sort simultaneously
    size_t k = nbytes / sizeof(data_type_t);
    data_type_t* buf = new data_type_t[k];
    if (buf == nullptr) {
        return -1;
    }
    int inp_fd = open(unsorted_file.c_str(), O_RDONLY);

    // read large input file chunk by chunk
    while (n > 0) {
        clock_t t1 = clock(), t2;
        size_t chunk_size = (n < k) ? n : k;
        n -= chunk_size;

        // read & sort
        printf("sorting chunk of %lu numbers (%lu MB). ", chunk_size,
            chunk_size * sizeof(data_type_t) / (1024 * 1024));
        read(inp_fd, buf, chunk_size * sizeof(data_type_t));
        qsort(buf, chunk_size, sizeof(data_type_t), data_less);
        t2 = clock();
        printf("sorted in %.2f seconds. ", (double)(t2 - t1) / CLOCKS_PER_SEC);

        // write results to temporary file and remember it's name
        char tmp_name[] = "/tmp/extsortXXXXXX";
        int out_fd = mkstemp(tmp_name);
        write(out_fd, buf, chunk_size * sizeof(data_type_t));
        close(out_fd);
        chunks_filenames.push(string(tmp_name));
        printf("written to %s\n", tmp_name);
    }

    delete[] buf;
    close(inp_fd);

    return 0;
}

// This functions performs 'elementary' merging operation.
// It takes MAXWAYS (or less) input files from "inputs", merges it and pushes
// file name of the resulting (sorted) file to the "outputs"
// Returns: 0 if success, else -1
// NOTE: processed input files will be removed from the 'inputs' queue

int merge(queue<string>& inputs, queue<string>& outputs, data_type_t* buf, size_t buf_size) {
    vector<InputWay*> inp_files;
    size_t k = (inputs.size() > MAXWAYS) ? MAXWAYS : inputs.size();
    size_t way_size = buf_size / (k + 1);
    size_t i = 0;

    // Create k input buffers (so-called input ways) and 1 output buffer (output way)
    printf("%lu-way merge [ ", k);
    for (i = 0; i < k; i++) {
        string input_name = inputs.front();
        printf("%s ", input_name.c_str());
        inp_files.push_back(new InputWay(input_name.c_str(), buf + i * way_size, way_size));
        inputs.pop();
    }
    printf("] ");

    OutputWay out(buf + i * way_size, way_size);

    while (true) {
        // find max element
        data_type_t start_value = DATA_MIN_VALUE;
        data_type_t max = start_value;
        InputWay* max_way = nullptr;

        for (size_t i = 0; i < k; i++) {
            data_type_t xi = inp_files[i]->top();
            if (!inp_files[i]->empty() && data_less(&max, &xi)) {
                max = xi;
                max_way = inp_files[i];
            }
        }
        if (max_way == nullptr) { // finished
            break;
        }

        // write max element to the output buffer
        out.put(max_way->get());
    }

    outputs.push(out.get_file_name());
    printf("-> %s\n", out.get_file_name().c_str());

    // cleanup
    for (i = 0; i < k; i++) {
        delete inp_files[i];
    }
}
