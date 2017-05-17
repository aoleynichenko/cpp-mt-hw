#include <string.h>

#include <coroutine/channel.h>

namespace Coroutine {

Channel::Channel(size_t buf_sz)
    :   buf_size(buf_sz), start(0), end(0)
{
    buf = new char[buf_size];
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

// returns number of bytes read
ssize_t Channel::read(void *to, size_t nbytes)
{
    if (nbytes == 0) {
        return 0;
    }

    if (start <= end) {
        size_t sz = end - start;
        size_t n_read = (nbytes <= sz) ? nbytes : sz;
        memcpy(to, buf + start, n_read);
        start += n_read;
        return n_read;
    }
    else {
        size_t sz_1 = buf_size - start;
        size_t n_read_1 = (nbytes <= sz_1) ? nbytes : sz_1;
        size_t n_read_2 = 0;

        memcpy(to, buf + start, n_read_1);
        start += n_read_1;
        nbytes -= n_read_1;

        if (nbytes > 0) {
            n_read_2 = (nbytes <= end) ? nbytes : end;
            memcpy((char*) to + n_read_1, buf, n_read_2);
            start = n_read_2;
        }
        return n_read_1 + n_read_2;
    }
}

// returns number of bytes written
ssize_t Channel::write(const void *from, size_t nbytes)
{
    if (nbytes == 0) {
        return 0;
    }

    if (end < start) {
        size_t sz = start - end - 1;
        size_t n_write = (nbytes <= sz) ? nbytes : sz;
        memcpy(buf + end, from, n_write);
        end += n_write;
        return n_write;
    }
    else {
        size_t sz_1 = buf_size - end;
        size_t n_write_1 = (nbytes <= sz_1) ? nbytes : sz_1;
        size_t n_write_2 = 0;

        memcpy(buf + end, from, n_write_1);
        end += n_write_1;
        nbytes -= n_write_1;

        if (nbytes > 0 && start > 0) {
            size_t sz_2 = start - 1;
            n_write_2 = (nbytes <= sz_2) ? nbytes : sz_2;
            memcpy(buf, (char*) from + n_write_1, n_write_2);
            if (n_write_2 != 0) {
                end = n_write_2;
            }
        }
        return n_write_1 + n_write_2;
    }
}


} // namespace Coroutine
