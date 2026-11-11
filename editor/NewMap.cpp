#include "editor/NewMap.h"

#include "editor/Areas.h"
#include "editor/Passages.h"

namespace maps = stardew::maps;

namespace {
// El tamano de la granja y el pueblo de valle.toml
constexpr AreaSize AREA_SIZE{30, 20};
}  // namespace

maps::MapDefinition newMap(const std::string& name, int formatVersion) {
    maps::MapDefinition map;
    map.name = name;
    map.formatVersion = formatVersion;
    addArea(map, maps::AreaType::Farm, AREA_SIZE, {0, 0});
    addArea(map, maps::AreaType::Village, AREA_SIZE, {AREA_SIZE.width, 0});
    // La ultima columna de la granja, a mitad de altura: el borde con el pueblo
    togglePassage(map, AREA_SIZE.width - 1, AREA_SIZE.height / 2);
    return map;
}
