#pragma once

#include <cstdint>

#include "client/menu/styles/CharacterColors.h"

// Cantidad de opciones de cada parte del personaje. Los ids de CharacterAppearance van de
// 0 a COUNT - 1. Provisorio: cuando esten los sprites, el catalogo tiene que coincidir con
// el que valida el servidor (si no, responde InvalidAppearance).
namespace AppearanceCatalog {
constexpr uint16_t HAIR_STYLE_COUNT = 10;
constexpr uint8_t HAIR_COLOR_COUNT = CharacterColors::PALETTE.size();
constexpr uint16_t SHIRT_COUNT = 30;
constexpr uint16_t PANTS_COUNT = 10;
// Incluye el 0, que es "sin sombrero".
constexpr uint16_t HAT_COUNT = 10;
}  // namespace AppearanceCatalog
