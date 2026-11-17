#include "client/gui/mapa/transicion_area.h"

#include <chrono>

#include <gtest/gtest.h>

namespace {

using std::chrono::milliseconds;

class TransicionAreaTest: public ::testing::Test {
protected:
    TransicionArea transicion{milliseconds(400)};
    std::chrono::steady_clock::time_point inicio;
};

}  // namespace

TEST_F(TransicionAreaTest, SinEmpezarNoOscureceNada) {
    EXPECT_FALSE(transicion.activa(inicio));
    EXPECT_EQ(transicion.opacidad(inicio), 0);
}

TEST_F(TransicionAreaTest, LaPrimeraMitadOscurece) {
    transicion.empezar(inicio);
    EXPECT_TRUE(transicion.oscureciendo(inicio + milliseconds(100)));
    EXPECT_NEAR(transicion.opacidad(inicio + milliseconds(100)), 127, 1);
}

TEST_F(TransicionAreaTest, EnLaMitadEstaTodoNegro) {
    transicion.empezar(inicio);
    EXPECT_NEAR(transicion.opacidad(inicio + milliseconds(199)), 255, 2);
}

TEST_F(TransicionAreaTest, LaSegundaMitadAclara) {
    transicion.empezar(inicio);
    EXPECT_FALSE(transicion.oscureciendo(inicio + milliseconds(300)));
    EXPECT_NEAR(transicion.opacidad(inicio + milliseconds(300)), 127, 1);
}

TEST_F(TransicionAreaTest, AlTerminarNoQuedaNegro) {
    transicion.empezar(inicio);
    EXPECT_FALSE(transicion.activa(inicio + milliseconds(400)));
    EXPECT_EQ(transicion.opacidad(inicio + milliseconds(400)), 0);
}
