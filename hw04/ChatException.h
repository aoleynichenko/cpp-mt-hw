#ifndef CHAT_EXCEPTION_H_INCLUDED
#define CHAT_EXCEPTION_H_INCLUDED

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>

class ChatException : public std::runtime_error {
public:
    ChatException(int sock_fd, const std::string& message, bool use_errno = false);

    virtual const char* what() const throw();

    int get_socket() const;

private:
    int socket_fd;
};

#endif // CHAT_EXCEPTION_H_INCLUDED
