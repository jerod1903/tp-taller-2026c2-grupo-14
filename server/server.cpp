#include "server.h"

#include <string>

#include "common/logger.h"

#include "consts.h"

Server::Server(const char* servicename, std::istream& input): input(input), acceptor(servicename) {}

void Server::execute() {
    acceptor.start();

    std::string line;
    while (std::getline(input, line) && line != ServerConstants::EXIT_COMMAND) {}
    Logger::info("cerrando servidor");
}
