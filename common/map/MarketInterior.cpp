#include "common/map/MarketInterior.h"

#include "common/map/InteriorWalls.h"

namespace stardew::maps {

MapDefinition loadMarketInterior() { return loadInterior(MARKET_INTERIOR_FILE); }

}  // namespace stardew::maps
