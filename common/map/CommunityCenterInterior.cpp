#include "common/map/CommunityCenterInterior.h"

#include "common/map/InteriorWalls.h"

namespace stardew::maps {

MapDefinition loadCommunityCenterInterior() { return loadInterior(COMMUNITY_CENTER_INTERIOR_FILE); }

}  // namespace stardew::maps
