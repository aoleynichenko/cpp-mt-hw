#include <errno.h>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string>

#include "ChatException.h"

ChatException::ChatException(int sock_fd, const std::string& message, bool use_errno)
    : std::runtime_error(message + ((use_errno == true) ? strerror(errno) : ""))
    , socket_fd(sock_fd) {
}

const char* ChatException::what() const throw() {
    std::ostringstream err_msg;

    err_msg << "on socket " << socket_fd << std::runtime_error::what();

    return err_msg.str().c_str();
}

int ChatException::get_socket() const {
    return socket_fd;
}
