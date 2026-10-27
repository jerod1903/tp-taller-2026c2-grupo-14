#include "server/server.h"

#include <string>
#include <utility>

#include "common/logger.h"
#include "server/consts.h"

Server::Server(const char* servicename, std::istream& input, std::vector<WorldInfo> worlds):
        input(input),
        registry(std::move(worlds), store, ServerConstants::MAX_PLAYERS),
        acceptor(servicename, registry) {}

void Server::execute() {
    acceptor.start();

    std::string line;
    while (std::getline(input, line) && line != ServerConstants::EXIT_COMMAND) {}
    Logger::info("cerrando servidor");
}
