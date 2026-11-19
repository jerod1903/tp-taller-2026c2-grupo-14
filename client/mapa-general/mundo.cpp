#include "client/mapa-general/mundo.h"

#include <stdexcept>
#include <string>

#include "common/map/persistence/MapRepository.h"

stardew::maps::MapDefinition cargar_mundo(const std::string& world_id) {
    // Con otros caracteres (una barra, un punto) podría leer un archivo fuera de la carpeta
    if (world_id.empty() || world_id.find_first_not_of(CARACTERES_WORLD_ID) != std::string::npos) {
        throw std::runtime_error("world id invalido: " + world_id);
    }
    stardew::maps::TomlMapRepository repositorio;
    return repositorio.loadMap(std::string(TALLER_MAPS_DIR) + "/" + world_id + ".toml");
}
