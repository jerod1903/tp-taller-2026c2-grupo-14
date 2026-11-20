#include "server/server.h"

#include <string>
#include <utility>
#include <vector>

#include "common/logger.h"
#include "server/config/server_config.h"
#include "server/consts.h"

Server::Server(const char* servicename, std::istream& input, const std::vector<GameWorld>& worlds):
        input(input),
        store(TALLER_SAVES_DIR),
        registry(worlds, store, server_config().game.max_players),
        acceptor(servicename, registry) {}

void Server::execute() {
    acceptor.start();

    std::string line;
    while (std::getline(input, line) && line != ServerConstants::EXIT_COMMAND) {}
    Logger::info("cerrando servidor");
}
