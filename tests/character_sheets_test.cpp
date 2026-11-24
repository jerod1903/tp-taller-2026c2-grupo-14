#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "common/character/CharacterSheets.h"

namespace sheets = stardew::character;

TEST(CharacterSheetsTest, ElPeloEstaEnSuCeldaDeLaGrilla) {
    EXPECT_EQ(sheets::hair_cell(0), (sheets::Point{0, 0}));
    EXPECT_EQ(sheets::hair_cell(3), (sheets::Point{48, 0}));
    // 16 por fila: el 17 es el segundo de la segunda fila
    EXPECT_EQ(sheets::hair_cell(17), (sheets::Point{16, 96}));
}

TEST(CharacterSheetsTest, LaCamisaYElPantalonEstanEnSuCelda) {
    EXPECT_EQ(sheets::shirt_cell(5), (sheets::Point{40, 0}));
    EXPECT_EQ(sheets::shirt_cell(16), (sheets::Point{0, 32}));
    // 10 bloques de 192x688 por fila
    EXPECT_EQ(sheets::pants_block(1), (sheets::Point{192, 0}));
    EXPECT_EQ(sheets::pants_block(12), (sheets::Point{384, 688}));
}

TEST(CharacterSheetsTest, ElSombreroNEstaEnLaCeldaNMenosUno) {
    EXPECT_EQ(sheets::hat_cell(1), (sheets::Point{0, 0}));
    EXPECT_EQ(sheets::hat_cell(2), (sheets::Point{20, 0}));
    EXPECT_EQ(sheets::hat_cell(13), (sheets::Point{0, 80}));
}

TEST(CharacterSheetsTest, LasMangasSonLosTresColoresMasUsadosDeClaroAOscuro) {
    const uint32_t oscuro = 0xFF101010;
    const uint32_t medio = 0xFF808080;
    const uint32_t claro = 0xFFF0F0F0;
    const uint32_t raro = 0xFF00FF00;
    // El oscuro es el mas usado, pero va ultimo por ser el mas oscuro; el raro no entra
    std::vector<uint32_t> camisa = {oscuro, oscuro, oscuro, medio, medio, claro, claro, raro};
    EXPECT_EQ(sheets::sleeve_colors_of(camisa),
              (std::vector<uint32_t>{0xF0F0F0, 0x808080, 0x101010}));
}

TEST(CharacterSheetsTest, LasMangasIgnoranLosPixelesTransparentes) {
    std::vector<uint32_t> camisa = {0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0xFF336699};
    EXPECT_EQ(sheets::sleeve_colors_of(camisa), (std::vector<uint32_t>{0x336699}));
}
