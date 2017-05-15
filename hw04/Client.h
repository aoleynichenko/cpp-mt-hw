#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <queue>
#include <stddef.h>
#include <sys/epoll.h>

class Client {
public:
    Client();
    Client(const Client&) = delete;
    Client(Client&&);
    ~Client();

    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&);

    void write_out(char* buf, size_t len);

    void flush();

    static Client create(int epoll_fd, int client_sock_fd);

private:

    struct WriteTask {
        char* buf;
        size_t size;
        size_t written;
    };

    int epoll_fd;
    int socket;
    std::queue<WriteTask> output;
};

#endif // CLIENT_H_INCLUDED
