#include "client/gui/sprites/herramienta_en_mano.h"

#include <tuple>

#include <gtest/gtest.h>

#include "client/gui/sprites/constantes_herramienta_en_mano.h"

TEST(HerramientaEnManoTest, SoloSeSostienenHerramientas) {
    EXPECT_TRUE(herramienta_en_mano_de(ItemType::Pickaxe, Orientacion::Abajo, 13));
    EXPECT_TRUE(herramienta_en_mano_de(ItemType::WateringCan, Orientacion::Abajo, 12));
    EXPECT_FALSE(herramienta_en_mano_de(ItemType::Milk, Orientacion::Abajo, 16));
    EXPECT_FALSE(herramienta_en_mano_de(ItemType::Empty, Orientacion::Abajo, 16));
}

TEST(HerramientaEnManoTest, LaEmpunaduraQuedaEnLaMano) {
    const auto& [mano_x, mano_y, espejada, detras] = POSICIONES_EN_MANO.at(Orientacion::Derecha);
    auto [agarre_x, agarre_y] = EMPUNADURAS.at(ItemType::Pickaxe);
    HerramientaEnMano pico = *herramienta_en_mano_de(ItemType::Pickaxe, Orientacion::Derecha, 13);
    EXPECT_EQ(pico.dx + agarre_x, mano_x);
    EXPECT_EQ(pico.dy + agarre_y, mano_y);
}

TEST(HerramientaEnManoTest, EspejadaLaEmpunaduraTambienSeEspeja) {
    const auto& [mano_x, mano_y, espejada, detras] =
            POSICIONES_EN_MANO.at(Orientacion::Izquierda);
    auto [agarre_x, agarre_y] = EMPUNADURAS.at(ItemType::Shovel);
    HerramientaEnMano pala = *herramienta_en_mano_de(ItemType::Shovel, Orientacion::Izquierda, 14);
    EXPECT_TRUE(pala.espejada);
    EXPECT_EQ(pala.dx + (14 - 1 - agarre_x), mano_x);
}

TEST(HerramientaEnManoTest, MirandoArribaQuedaDetras) {
    EXPECT_TRUE(herramienta_en_mano_de(ItemType::Sword, Orientacion::Arriba, 14)->detras);
    EXPECT_FALSE(herramienta_en_mano_de(ItemType::Sword, Orientacion::Abajo, 14)->detras);
}
