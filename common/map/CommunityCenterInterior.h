#pragma once

#include "common/map/MapDefinition.h"

// El interior del centro comunitario es el mismo en todos los mundos: sale de
// maps/interiors/community_center.toml, con un objeto por set donde se entrega. Lo usan el
// servidor (para armarlo) y el cliente (para dibujarlo).
namespace stardew::maps {

inline constexpr const char* COMMUNITY_CENTER_INTERIOR_FILE =
        TALLER_MAPS_DIR "/interiors/community_center.toml";

// Lee el interior y arma las paredes. Lanza std::runtime_error si el archivo no se puede leer o no
// tiene exactamente un area
MapDefinition loadCommunityCenterInterior();

}  // namespace stardew::maps
