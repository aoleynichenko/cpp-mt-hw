#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <sys/epoll.h>
#include <queue>
#include <stddef.h>

class Client {
public:
    Client(int efd, int client_sock_fd);

    Client();
    Client(const Client&) = delete;
    Client(Client&&);
    ~Client();

    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&);

    void write_out(char* buf, size_t len);

    void flush();
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
