// Very simple Epoll chat - SERVER
//---------------------------------
// This solution is based on the following code:
// https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
// I used this example only in order to clearly understand how to use Epoll and
// can explain each line of the "borrowed" code.
//
// Client application is not provided (however, it can be easily implemented);
// please, use Telnet instead.
//
// Example:
// server:   ./chat-server 12345
// client:   telnet localhost 12345
//
// 02 April 2017
// Alexander Oleynichenko
// ao2310@yandex.ru

// EPOLLIN  - new data in socket to be read
// EPOLLOUT - socket is ready to continue data receiving
// EPOLLERR - error in socket (file descriptor)
// EPOLLHUP - socket was closed

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <vector>

#include "ChatException.h"
#include "Client.h"

#define MAXEVENTS 64
#define MAX_MSG_CHUNK 512

int create_master_socket(char* port_str);
int set_nonblocking(int sock_fd);

// creates socket on port specified in port_str
// returns: socket descriptor or -1 if an error occured
int create_master_socket(char* port_str) {
    int sock_fd;
    struct sockaddr_in sock_addr;
    uint32_t port;

    port = strtol(port_str, nullptr, 10);

    bzero(&sock_addr, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int s = bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (s == -1) {
        perror("bind");
        return -1;
    }

    s = set_nonblocking(sock_fd);
    if (s == -1) {
        perror("set_nonblocking");
        return -1;
    }

    return sock_fd;
}

int set_nonblocking(int sock_fd) {
    int flags = fcntl(sock_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;

    int s = fcntl(sock_fd, F_SETFL, flags);
    if (s == -1) {
        perror("fcntl");
        return -1;
    }
    return 0;
}

int main(int argc, char** argv) {
    int master_socket;
    std::map<int, Client> clients; // map: socket_fd -> Client object

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        return 1;
    }

    master_socket = create_master_socket(argv[1]);
    if (master_socket == -1) {
        return 1;
    }

    // listen port
    int s = listen(master_socket, SOMAXCONN);
    if (s == -1) {
        perror("listen");
        return 1;
    }

    // data for epoll
    int efd = epoll_create1(0);
    struct epoll_event event;
    struct epoll_event* events;

    event.data.fd = master_socket;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, master_socket, &event);
    if (s == -1) {
        perror("epoll_ctl");
        return 1;
    }
    events = new epoll_event[MAXEVENTS];
    printf("Epoll started successfully\n");

    while (true) {
        int n = epoll_wait(efd, events, MAXEVENTS, -1);
        for (int i = 0; i < n; i++) {
            try {
                int fd = events[i].data.fd;
                // error in socket or it was closed
                if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & (EPOLLIN | EPOLLOUT)))) {
                    printf("epoll error at descriptor %d\n", fd);
                    clients.erase(fd);
                    continue;
                }
                if ((events[i].events & EPOLLOUT) && (fd != master_socket)) {
                    Client& c = clients[fd];
                    // flush next chunk of data to the network
                    // here an object on ChatException can be thrown
                    c.flush();
                }
                // process incoming connections
                else if (fd == master_socket) {
                    while (true) {
                        struct sockaddr in_addr;
                        socklen_t in_len;
                        int infd;
                        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                        in_len = sizeof(in_addr);
                        infd = accept(master_socket, &in_addr, &in_len);

                        if (infd == -1) {
                            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                                break; // no incoming connections remain
                            } else {
                                perror("accept\n");
                                break;
                            }
                        }

                        clients[infd] = Client(efd, infd);
                    }
                    continue;
                }
                // new message was received
                else {
                    int done = 0;
                    // read input message chunk-by-chunk
                    while (true) {
                        ssize_t count;
                        char buf[MAX_MSG_CHUNK];
                        count = read(fd, buf, sizeof(buf));
                        if (count == -1) {
                            if (errno != EAGAIN) {
                                perror("read");
                                done = 1;
                            }
                            break;
                        } else if (count == 0) { // EOF; connection closed
                            done = 1;
                            break;
                        }

                        for (std::pair<const int, Client>& kv : clients) {
                            // here an object on ChatException can be thrown
                            kv.second.write_out(buf, count); // this operation is non-blocking!
                        }
                    }
                    if (done) {
                        clients.erase(fd);
                    }
                }
            } catch (ChatException& ex) {
                printf("Error: %s\n", ex.what());
                close(ex.get_socket());
            }
        }
    }

    delete[] events;
    close(master_socket);
}
