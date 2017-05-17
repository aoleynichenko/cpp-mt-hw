#include <string.h>

#include <coroutine/channel.h>

namespace Coroutine {

Channel::Channel(size_t buf_sz)
    :   buf_size(buf_sz), start(0), end(0)
{
    buf = new char[buf_size];
    for (auto i = 0; i < buf_size; i++)
        buf[i] = '_';
}

Channel::Channel(Channel&& other)
{
    buf_size = other.buf_size;
    buf = other.buf;
    start = other.start;
    end = other.end;

    other.buf_size = 0;
    other.buf = nullptr;
    other.start = 0;
    other.end = 0;
}

Channel& Channel::operator=(Channel&& other)
{
    if (this != &other) {
        buf_size = other.buf_size;
        buf = other.buf;
        start = other.start;
        end = other.end;

        other.buf_size = 0;
        other.buf = nullptr;
        other.start = 0;
        other.end = 0;
    }
}

Channel::~Channel()
{
    delete buf;
}

#include <stdio.h>

int Channel::read(void *to, size_t nbytes)
{
    if (nbytes == 0) {
        return 0;
    }

    if (start <= end) {
        printf("1-step read\n");
        size_t has_bytes = end - start;
        size_t n_read = (nbytes <= has_bytes) ? nbytes : has_bytes;
        memcpy(to, buf+start, n_read);
        start += n_read;
        return n_read;
    }
    else {
        printf("2-step read\n");
        size_t has_bytes_1 = buf_size - start;
        printf("has_bytes_1 = %lu\n", has_bytes_1);
        size_t n_read = (nbytes <= has_bytes_1) ? nbytes : has_bytes_1;
        memcpy(to, buf+start, n_read);
        start += n_read;
        if (n_read == has_bytes_1) {
            size_t has_bytes_2 = end;
            start = 0;
            nbytes -= n_read;
            n_read = (nbytes <= has_bytes_2) ? nbytes : has_bytes_2;
            memcpy(to+has_bytes_1, buf+start, n_read);
            start += n_read;
            n_read += has_bytes_1;
        }
        return n_read;
    }
}

int Channel::write(const void *from, size_t nbytes)
{
    if (nbytes == 0) {
        return 0;
    }

    printf("from = |%s|\n", from);

    if (end < start) {
        printf("1-step write\n");
        size_t size = start - end - 1;
        size_t n_write = (nbytes <= size) ? nbytes : size;
        memcpy(buf+end, from, n_write);
        end += n_write;
        return n_write;
    }
    else {
        printf("2-step write\n");
        size_t size_1 = buf_size - end;
        size_t n_write = (nbytes <= size_1) ? nbytes : size_1;
        printf("size_1 = %lu   n_write_1 = %lu\n", size_1, n_write);

        //dump();

        memcpy(buf+end, from, n_write);
        end += n_write;

        //dump();

        if (n_write == size_1) {
            //end = 0;
            size_t size_2 = start;
            printf("size_2 = %lu\n", size_2);
            nbytes -= n_write;
            n_write = (nbytes <= size_2) ? nbytes : size_2;
            printf("n_write_2 = %lu\n", n_write);
            memcpy(buf, from, n_write);
            end = (n_write == 0) ? end : n_write;
        }
        return size_1 + n_write;
    }
}



void Channel::dump()
{
    printf("-------------------------------------------------------\n");
    for (size_t i = 0; i < buf_size; i++) {
        printf("|%c", buf[i]);
    }
    printf("|\n");
    for (size_t i = 0; i < buf_size*2+2; i++) {
        if (i == start*2+1)
          printf("s");
        else if (i == end*2+1)
          printf("e");
        else
          printf(" ");
    }
    printf("\nstart = %d\n", start);
    printf("end = %d\n", end);
    printf("-------------------------------------------------------\n");
}

} // namespace Coroutine
