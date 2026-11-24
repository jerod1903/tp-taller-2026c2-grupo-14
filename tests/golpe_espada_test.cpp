#include "client/gui/sprites/golpe_espada.h"

#include <tuple>

#include <gtest/gtest.h>

#include "client/gui/sprites/constantes_golpe_espada.h"

TEST(GolpeEspadaTest, EnLaMitadApuntaAlMedioDelBarrido) {
    GolpeEspada golpe = golpe_espada_de(Orientacion::Derecha, 0.5f);
    EXPECT_DOUBLE_EQ(golpe.angulo, std::get<0>(POSICIONES_GOLPE_ESPADA.at(Orientacion::Derecha)));
}

TEST(GolpeEspadaTest, BarreDeUnLadoAlOtro) {
    double medio = std::get<0>(POSICIONES_GOLPE_ESPADA.at(Orientacion::Abajo));
    EXPECT_DOUBLE_EQ(golpe_espada_de(Orientacion::Abajo, 0.0f).angulo,
                     medio - BARRIDO_GOLPE_ESPADA / 2);
    EXPECT_DOUBLE_EQ(golpe_espada_de(Orientacion::Abajo, 1.0f).angulo,
                     medio + BARRIDO_GOLPE_ESPADA / 2);
}

TEST(GolpeEspadaTest, MirandoArribaQuedaDetras) {
    EXPECT_TRUE(golpe_espada_de(Orientacion::Arriba, 0.5f).detras);
    EXPECT_FALSE(golpe_espada_de(Orientacion::Abajo, 0.5f).detras);
}

TEST(GolpeEspadaTest, IzquierdaEsElEspejoDeDerecha) {
    GolpeEspada derecha = golpe_espada_de(Orientacion::Derecha, 0.5f);
    GolpeEspada izquierda = golpe_espada_de(Orientacion::Izquierda, 0.5f);
    EXPECT_EQ(izquierda.dx, -derecha.dx);
    EXPECT_EQ(izquierda.dy, derecha.dy);
}
