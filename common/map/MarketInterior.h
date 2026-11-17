#pragma once

#include "common/map/MapDefinition.h"

// El interior del mercado es el mismo en todos los mundos: sale de maps/interiors/market.toml, con
// los comerciantes adentro. Lo usan el servidor (para armar el Market) y el cliente (para
// dibujarlo).
namespace stardew::maps {

inline constexpr const char* MARKET_INTERIOR_FILE = TALLER_MAPS_DIR "/interiors/market.toml";

// Lee el interior y arma las paredes. Lanza std::runtime_error si el archivo no se puede leer o no
// tiene exactamente un area
MapDefinition loadMarketInterior();

}  // namespace stardew::maps
