#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <queue>

#include "ChatException.h"
#include "Client.h"

using std::queue;

#define VOID_SOCKET -1

int set_nonblocking(int sock_fd);

Client::Client(int efd, int client_sock_fd)
    : epoll_fd(efd)
    , socket(client_sock_fd) {
    epoll_event event;

    set_nonblocking(socket);

    event.data.fd = socket;
    event.events = EPOLLIN | EPOLLET;
    int s = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event);
    if (s == -1) {
        throw ChatException(socket, "in Client::Client: ", true);
    }

    printf("Accepted connection on descriptor %d\n", socket);
}

Client::Client() {}

Client::Client(Client&& other)
    : epoll_fd(other.epoll_fd) {
    socket = other.socket;
    other.socket = VOID_SOCKET;
}

Client::~Client() {
    if (socket != VOID_SOCKET) {
        close(socket);
        printf("connection closed on descriptor %d\n", socket);
    }
}

Client& Client::operator=(Client&& other) {
    if (this != &other) {
        epoll_fd = other.epoll_fd;
        socket = other.socket;
        other.socket = VOID_SOCKET;
    }
    return *this;
}

void Client::write_out(char* buf, size_t len) {
    // if the socket was moved from this object
    if (socket == VOID_SOCKET) {
        throw ChatException(socket, "in Client::write_out(): the socket was moved");
    }

    // create and initialize write task
    WriteTask t;
    t.buf = new char[len];
    t.size = len;
    t.written = 0;
    strncpy(t.buf, buf, len);

    // push the output string to the waiting queue
    output.push(t);

    // tell epoll to listen for 'socket is writeable' events
    epoll_event event;
    event.data.fd = socket;
    event.events = EPOLLIN | EPOLLOUT;

    int s = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket, &event);
    if (s == -1) {
        throw ChatException(socket, "epoll_ctl in Client::write_out()", true);
    }
}

void Client::flush() {
    // if the socket was moved from this object
    if (socket == VOID_SOCKET) {
        throw ChatException(socket, "in Client::write_out(): the socket was moved");
    }

    WriteTask& t = output.front();
    size_t remain = t.size - t.written;

    int n_written = write(socket, t.buf + t.written, remain);
    if (n_written == -1) {
        throw ChatException(socket, "write in Client::flush()", true);
    }
    t.written += (size_t)n_written;

    // check: if this write task is finished or not?
    if (t.written == t.size) { // finished
        output.pop();
        delete[] t.buf;
    }

    // if all messages were sent, disable EPOLLOUT events to save CPU
    if (output.empty()) {
        epoll_event event;
        event.data.fd = socket;
        event.events = EPOLLIN;

        int s = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket, &event);
        if (s == -1) {
            throw ChatException(socket, "epoll_ctl in Client::flush()", true);
        }
    }
}
