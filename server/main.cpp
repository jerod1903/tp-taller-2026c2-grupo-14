#include <exception>
#include <iostream>
#include <utility>
#include <vector>

#include "common/map/persistence/MapRequirementsLoader.h"
#include "server/consts.h"
#include "server/game/world/world_loader.h"
#include "server/server.h"

int main(int argc, char* argv[]) {
    if (argc != ServerConstants::EXPECTED_ARGC) {
        std::cerr << ServerConstants::USAGE_PREFIX << argv[ServerConstants::ARG_PROGRAM_NAME]
                  << ServerConstants::USAGE_ARGS;
        return ServerConstants::EXIT_ERROR;
    }

    try {
        std::vector<WorldInfo> worlds;
        for (const GameWorld& world:
             load_worlds(TALLER_MAPS_DIR,
                         stardew::maps::loadMapRequirements(ServerConstants::MAP_REQUIREMENTS))) {
            worlds.push_back(world.info);
        }
        Server server(argv[ServerConstants::ARG_SERVICENAME], std::cin, std::move(worlds));
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
