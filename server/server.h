#ifndef SERVER_H
#define SERVER_H

#include <istream>
#include <vector>

#include "common/lobby_dto.h"

#include "acceptor.h"
#include "game_registry.h"
#include "in_memory_game_store.h"

class Server {
public:
    Server(const char* servicename, std::istream& input, std::vector<WorldInfo> worlds);

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void execute();

private:
    std::istream& input;
    // Orden de declaración = orden de construcción: el acceptor usa al registry, y este al store.
    InMemoryGameStore store;
    GameRegistry registry;
    Acceptor acceptor;
};

#endif  // SERVER_H
