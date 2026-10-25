#ifndef SERVER_H
#define SERVER_H

#include <istream>

#include "acceptor.h"

class Server {
public:
    Server(const char* servicename, std::istream& input);

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void execute();

private:
    std::istream& input;
    Acceptor acceptor;
};

#endif  // SERVER_H
