#ifndef COROUTINE_CHANNEL_H
#define COROUTINE_CHANNEL_H

#include <stddef.h>

#define DEFAULT_CHANNEL_BUF_SIZE 4096

namespace Coroutine {

class Channel {
public:
    Channel(size_t buf_sz = DEFAULT_CHANNEL_BUF_SIZE);

    Channel(const Channel& other) = delete;
    Channel(Channel&& other);

    Channel& operator=(const Channel& other);
    Channel& operator=(Channel&& other);

    ~Channel();

    int read(void *buf, size_t nbyte);
    int write(const void *buf, size_t nbyte);

    void dump();
private:
    char* buf;
    size_t buf_size;
    int start;
    int end;
};

} // namespace Coroutine

#endif // COROUTINE_CHANNEL_H
