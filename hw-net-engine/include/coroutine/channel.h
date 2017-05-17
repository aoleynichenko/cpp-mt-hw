#ifndef COROUTINE_CHANNEL_H
#define COROUTINE_CHANNEL_H

#include <cstddef>
#include <cstdlib>

#define DEFAULT_CHANNEL_BUF_SIZE 4096

namespace Coroutine {

/**
 *  This class implements ring buffer
 */
class Channel {
public:
    Channel(size_t buf_sz = DEFAULT_CHANNEL_BUF_SIZE);

    Channel(const Channel& other) = delete;
    Channel(Channel&& other);

    Channel& operator=(const Channel& other);
    Channel& operator=(Channel&& other);

    ~Channel();

    // returns number of bytes read
    ssize_t read(void *buf, size_t nbyte);

    // returns number of bytes written
    ssize_t write(const void *buf, size_t nbyte);

private:
    char* buf;
    size_t buf_size;
    size_t start;
    size_t end;
};

} // namespace Coroutine

#endif // COROUTINE_CHANNEL_H
