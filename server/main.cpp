#include <exception>
#include <iostream>

#include "consts.h"
#include "server.h"

int main(int argc, char* argv[]) {
    if (argc != ServerConstants::EXPECTED_ARGC) {
        std::cerr << ServerConstants::USAGE_PREFIX << argv[ServerConstants::ARG_PROGRAM_NAME]
                  << ServerConstants::USAGE_ARGS;
        return ServerConstants::EXIT_ERROR;
    }

    try {
        // Los mundos se van a cargar de la carpeta de mapas que produce el editor.
        Server server(argv[ServerConstants::ARG_SERVICENAME], std::cin, {});
        server.execute();
    } catch (const std::exception& e) {
        std::cerr << ServerConstants::ERROR_PREFIX << e.what() << '\n';
        return ServerConstants::EXIT_ERROR;
    } catch (...) {
        std::cerr << ServerConstants::UNKNOWN_ERROR << '\n';
        return ServerConstants::EXIT_ERROR;
    }

    return ServerConstants::EXIT_OK;
}
