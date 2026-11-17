#include "common/map/CabinInterior.h"

#include <stdexcept>
#include <vector>

#include "common/map/InteriorWalls.h"
#include "common/map/persistence/MapRepository.h"

namespace stardew::maps {

void addBedroom(MapArea& area, int maxPlayers) {
    // La ultima fila de piso de la sala, mirando su ultima columna antes de la pared
    int lastFloorRow = -1;
    for (int y = 0; y < area.height; ++y) {
        if (isWalkableTerrain(area.tiles.at(y).at(area.width - 2))) {
            lastFloorRow = y;
        }
    }
    // Las camas pisan el zocalo con su ultima fila
    const int bedRow = lastFloorRow + 2 - BED_HEIGHT;
    const int firstFloorRow = bedRow - BEDROOM_FREE_ROWS;
    if (firstFloorRow < BACK_WALL_ROWS) {
        throw std::invalid_argument("la cabaña es muy baja para el cuarto de las camas");
    }
    // Una casilla libre, las camas con su casilla libre y la pared de la derecha
    const int bedroomWidth = 1 + BEDROOM_COLUMNS_PER_BED * maxPlayers + 1;
    for (int y = 0; y < area.height; ++y) {
        const bool floorRow = y >= firstFloorRow && y <= lastFloorRow;
        for (int x = 0; x < bedroomWidth; ++x) {
            const bool floor = floorRow && x < bedroomWidth - 1;
            area.tiles.at(y).push_back(floor ? TileType::HouseFloor : TileType::Void);
        }
    }
    for (int bed = 0; bed < maxPlayers; ++bed) {
        area.tiles.at(bedRow).at(area.width + 1 + BEDROOM_COLUMNS_PER_BED * bed) =
                TileType::Sleepable;
    }
    area.width += bedroomWidth;
}

MapDefinition loadCabinInterior(int maxPlayers) {
    MapDefinition interior = TomlMapRepository().loadMap(CABIN_INTERIOR_FILE);
    if (interior.areas.size() != 1) {
        throw std::runtime_error("el interior de la cabaña tiene que tener una sola área");
    }
    addBedroom(interior.areas.front(), maxPlayers);
    addWalls(interior.areas.front());
    return interior;
}

}  // namespace stardew::maps
