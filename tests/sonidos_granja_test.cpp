#include "client/audio/sonidos_granja.h"

#include <cstdint>
#include <set>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace {

FarmTile celda(SoilType suelo, CropType cultivo = CropType::None, uint8_t etapa = 0,
               bool necesita_agua = false) {
    return FarmTile{3, 4, suelo, cultivo, etapa, necesita_agua, false};
}

FarmState granja(std::vector<FarmTile> celdas) { return FarmState{"granja", std::move(celdas), {}}; }

}  // namespace

TEST(SonidosGranjaTest, UnaCeldaNuevaEsAzada) {
    EXPECT_EQ(sonidos_granja(granja({}), granja({celda(SoilType::Dirt)})),
              std::set<Sonido>{Sonido::Azada});
}

TEST(SonidosGranjaTest, ArarEsAzada) {
    EXPECT_EQ(sonidos_granja(granja({celda(SoilType::Dirt)}), granja({celda(SoilType::Tilled)})),
              std::set<Sonido>{Sonido::Azada});
}

TEST(SonidosGranjaTest, PlantarEsPlantar) {
    EXPECT_EQ(sonidos_granja(granja({celda(SoilType::Tilled)}),
                             granja({celda(SoilType::Tilled, CropType::Potato)})),
              std::set<Sonido>{Sonido::Plantar});
}

TEST(SonidosGranjaTest, RegarEsRegar) {
    EXPECT_EQ(sonidos_granja(granja({celda(SoilType::Tilled, CropType::Potato, 2, true)}),
                             granja({celda(SoilType::Tilled, CropType::Potato, 2, false)})),
              std::set<Sonido>{Sonido::Regar});
}

TEST(SonidosGranjaTest, CrecerAlAmanecerNoSuena) {
    EXPECT_TRUE(sonidos_granja(granja({celda(SoilType::Tilled, CropType::Potato, 2, true)}),
                               granja({celda(SoilType::Tilled, CropType::Potato, 3, false)}))
                        .empty());
}

TEST(SonidosGranjaTest, CosecharNoSuenaEnLaTierra) {
    EXPECT_TRUE(sonidos_granja(granja({celda(SoilType::Tilled, CropType::Potato, 6)}),
                               granja({celda(SoilType::Tilled)}))
                        .empty());
}

TEST(SonidosGranjaTest, OtraAreaNoSuena) {
    FarmState otra{"pueblo", {celda(SoilType::Dirt)}, {}};
    EXPECT_TRUE(sonidos_granja(granja({}), otra).empty());
}
