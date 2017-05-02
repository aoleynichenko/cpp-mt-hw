#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <queue>

#include "Client.h"

using std::queue;


Client::Client(int efd, int client_sock_fd)
    :   epoll_fd(efd), socket(client_sock_fd)
{
}

Client::Client() {}

Client::Client(const Client&) {}

// returns 0 if success, else -1
// if an error occured, whe should simply shutdown connection
int Client::write_out(char* buf, size_t len)
{
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
    event.events = EPOLLIN | /*EPOLLET |*/ EPOLLOUT;

    int s = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket, &event);
    if (s == -1) {
        perror("epoll_ctl in Client::write()");
        return -1;
    }
    return 0;
}

int Client::flush()
{
    WriteTask& t = output.front();
    size_t remain = t.size - t.written;

    int n_written = write(socket, t.buf + t.written, remain);
    if (n_written == -1) {
        perror("write in Client::flush()");
        return -1;
    }
    t.written += (size_t) n_written;

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
            perror("epoll_ctl in Client::flush()");
            return -1;
        }
    }

    // success
    return 0;
}
