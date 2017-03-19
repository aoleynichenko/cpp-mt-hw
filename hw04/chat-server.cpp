#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int create_master_socket(char* port) {
  int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
