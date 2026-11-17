#include "common/map/MarketInterior.h"

#include <stdexcept>

#include "common/map/InteriorWalls.h"
#include "common/map/persistence/MapRepository.h"

namespace stardew::maps {

MapDefinition loadMarketInterior() {
    MapDefinition interior = TomlMapRepository().loadMap(MARKET_INTERIOR_FILE);
    if (interior.areas.size() != 1) {
        throw std::runtime_error("el interior del mercado tiene que tener una sola área");
    }
    addWalls(interior.areas.front());
    return interior;
}

}  // namespace stardew::maps
