#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int create_master_socket(char* port_str) {
  int s;
  struct sockaddr_in sock_addr;
  long int port;

  port = strtol(port_str, nullptr, 10);

  bzero(&sock_addr, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons(port);
  sock_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  return s;
}

int main (int argc, char** argv)
{
  if (argc != 2) {
      fprintf (stderr, "Usage: %s [port]\n", argv[0]);
      return 1;
    }
    int master_socket = create_master_socket(argv[1]);
    close(master_socket);
}
