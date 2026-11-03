#include "server/game/world/world_loader.h"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <optional>
#include <utility>

#include "common/logger.h"
#include "common/map/persistence/MapRepository.h"
#include "common/map/service/MapService.h"
#include "server/game/world/world_builder.h"

namespace fs = std::filesystem;
namespace maps = stardew::maps;

namespace {

bool is_map_file(const fs::directory_entry& entry) {
    return entry.is_regular_file() && entry.path().extension() == ".toml";
}

std::optional<GameWorld> load_world(const maps::MapService& service, const fs::path& path) {
    std::string file = path.filename().string();
    try {
        maps::MapLoadResult result = service.loadMap(path.string());
        if (!result.issues.empty()) {
            Logger::warn("mapa '{}' inválido: {} problemas, el primero {}", file,
                         result.issues.size(), maps::toString(result.issues.front().code));
            return std::nullopt;
        }
        build_world(result.map);
        return GameWorld{WorldInfo{path.stem().string(), result.map.name}, std::move(result.map)};
    } catch (const std::exception& e) {
        Logger::warn("mapa '{}' no se pudo cargar: {}", file, e.what());
        return std::nullopt;
    }
}

}  // namespace

std::vector<GameWorld> load_worlds(const std::string& maps_dir,
                                   const maps::MapRequirements& requirements) {
    std::vector<GameWorld> worlds;
    if (!fs::is_directory(maps_dir)) {
        Logger::warn("no existe la carpeta de mapas '{}'", maps_dir);
        return worlds;
    }
    maps::TomlMapRepository repository;
    maps::MapService service(repository, requirements);
    for (const fs::directory_entry& entry: fs::directory_iterator(maps_dir)) {
        if (!is_map_file(entry)) {
            continue;
        }
        std::optional<GameWorld> world = load_world(service, entry.path());
        if (world) {
            Logger::info("mundo '{}' cargado", world->info.worldId);
            worlds.push_back(std::move(*world));
        }
    }
    std::sort(worlds.begin(), worlds.end(), [](const GameWorld& a, const GameWorld& b) {
        return a.info.worldId < b.info.worldId;
    });
    return worlds;
}
