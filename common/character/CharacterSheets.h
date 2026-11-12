#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

// Formato de las hojas del granjero (las de Stardew Valley): donde esta cada prenda y como se
// superpone al cuerpo. Lo comparten el juego (SDL) y el preview del menu (Qt): por eso son solo
// numeros, sin tipos de ninguna de las dos librerias. Los colores van en 0xRRGGBB.
namespace stardew::character {

// Rutas de las hojas, relativas a la carpeta Assets/
inline const char* const SHEET_BODY =
        "Characters/StardewValley/StardewValley_Character_Farmer Base.png";
inline const char* const SHEET_HAIR =
        "Characters/StardewValley/StardewValley_Character_Hairstyles.png";
inline const char* const SHEET_SHIRTS =
        "Characters/StardewValley/StardewValley_Character_Shirts.png";
inline const char* const SHEET_PANTS = "Characters/StardewValley/StardewValley_Character_Pants.png";
inline const char* const SHEET_HATS = "Characters/StardewValley/StardewValley_Character_Hats.png";

// Un punto o un tamaño en pixeles de la hoja
struct Point {
    int x;
    int y;

    bool operator==(const Point&) const = default;
};

// Cuerpo: cuadros de 16x32, una fila por orientacion. Los brazos estan en la misma hoja, 6 cuadros
// a la derecha del cuerpo, y se dibujan aparte, encima de la camisa
inline constexpr Point FRAME_SIZE{16, 32};
inline constexpr int ARMS_COLUMNS = 6;
// Los brazos traen las mangas con estos tres colores (de claro a oscuro), que se reemplazan por los
// de la camisa
inline constexpr std::array<uint32_t, 3> SLEEVE_COLORS = {0x8E1F0C, 0x701718, 0x4A0C06};

// Pelo: celdas de 16x96 con tres vistas de 16x32 (frente, costado, espalda), 16 por fila. Viene en
// gris para teñirlo con el color elegido
inline constexpr Point HAIR_CELL{16, 96};
inline constexpr int HAIR_PER_ROW = 16;
inline constexpr Point HAIR_OFFSET{0, 1};

// Camisa: celdas de 8x32 con cuatro vistas de 8x8 (frente, derecha, izquierda, espalda), 16 por
// fila. Va sobre el pecho
inline constexpr Point SHIRT_SIZE{8, 8};
inline constexpr Point SHIRT_CELL{8, 32};
inline constexpr int SHIRTS_PER_ROW = 16;
inline constexpr Point SHIRT_OFFSET{4, 14};

// Pantalon: un bloque de 192x688 por pantalon, 10 por fila, con los mismos cuadros que el cuerpo.
// Viene en gris: se tiñe con un color fijo porque la apariencia no elige color
inline constexpr Point PANTS_BLOCK{192, 688};
inline constexpr int PANTS_PER_ROW = 10;
inline constexpr uint32_t PANTS_COLOR = 0x2E55B7;

// Sombrero: celdas de 20x80 con cuatro vistas de 20x20 (frente, derecha, izquierda, espalda), 12
// por fila. Sobresale de la cabeza. El 0 es "sin sombrero": el sombrero n esta en la celda n - 1
inline constexpr Point HAT_SIZE{20, 20};
inline constexpr Point HAT_CELL{20, 80};
inline constexpr int HATS_PER_ROW = 12;
inline constexpr Point HAT_OFFSET{-2, -3};
inline constexpr uint16_t NO_HAT = 0;

// Esquina de la celda de cada prenda dentro de su hoja
inline constexpr Point cell_of(uint16_t id, int per_row, Point cell) {
    return Point{(id % per_row) * cell.x, (id / per_row) * cell.y};
}
inline constexpr Point hair_cell(uint16_t hair_style) {
    return cell_of(hair_style, HAIR_PER_ROW, HAIR_CELL);
}
inline constexpr Point shirt_cell(uint16_t shirt) {
    return cell_of(shirt, SHIRTS_PER_ROW, SHIRT_CELL);
}
inline constexpr Point pants_block(uint16_t pants) {
    return cell_of(pants, PANTS_PER_ROW, PANTS_BLOCK);
}
// Solo para hat != NO_HAT
inline constexpr Point hat_cell(uint16_t hat) {
    return cell_of(static_cast<uint16_t>(hat - 1), HATS_PER_ROW, HAT_CELL);
}

// Aproximacion a como el juego elige el color de la manga: los tres colores mas usados en la
// camisa (en 0xAARRGGBB; los transparentes no cuentan), de claro a oscuro. Si tiene menos de tres,
// devuelve los que haya
inline std::vector<uint32_t> sleeve_colors_of(const std::vector<uint32_t>& shirt_pixels) {
    std::map<uint32_t, int> uses;
    for (uint32_t pixel: shirt_pixels) {
        if ((pixel >> 24) != 0) {
            ++uses[pixel & 0xFFFFFF];
        }
    }
    std::vector<std::pair<uint32_t, int>> by_use(uses.begin(), uses.end());
    std::stable_sort(by_use.begin(), by_use.end(),
                     [](const auto& a, const auto& b) { return a.second > b.second; });
    std::vector<uint32_t> colors;
    for (std::size_t i = 0; i < by_use.size() && i < SLEEVE_COLORS.size(); ++i) {
        colors.push_back(by_use[i].first);
    }
    // Claridad como en HSL: el promedio del canal mas alto y el mas bajo
    auto lightness = [](uint32_t color) {
        int r = (color >> 16) & 0xFF;
        int g = (color >> 8) & 0xFF;
        int b = color & 0xFF;
        return std::max({r, g, b}) + std::min({r, g, b});
    };
    std::stable_sort(colors.begin(), colors.end(),
                     [&lightness](uint32_t a, uint32_t b) { return lightness(a) > lightness(b); });
    return colors;
}

}  // namespace stardew::character
