#pragma once

#include <array>

// Paleta de colores del personaje (2 filas de 8). El indice es el que viaja en
// CharacterAppearance::hairColor; la va a usar tambien el selector de colores.
namespace CharacterColors {
constexpr std::array<const char*, 16> PALETTE = {
        "#141414", "#D2373C", "#2DB46E", "#E6E632", "#2D73C8", "#B43CB4", "#28A0C8", "#E6E6E6",
        "#5A5A5A", "#F06464", "#46D79B", "#FAFA5A", "#5A9BF0", "#DC78E6", "#5AC8E6", "#FFFFFF",
};
}  // namespace CharacterColors
