#include "client/gui/sprites/animador_sprites.h"

#include <chrono>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace {

using std::chrono::milliseconds;

// Como la hoja del granjero, con cuadros de 100 ms
const FormatoAnimacion FORMATO{16,
                               32,
                               {{Orientacion::Abajo, {0, false}},
                                {Orientacion::Arriba, {2, false}},
                                {Orientacion::Derecha, {1, false}},
                                {Orientacion::Izquierda, {1, true}}},
                               {0, 1, 0, 2},
                               milliseconds(100),
                               {{1, 1}, {2, 1}}};

class AnimadorSpritesTest: public ::testing::Test {
protected:
    AnimadorSprites animador{FORMATO};
    std::chrono::steady_clock::time_point inicio;
};

}  // namespace

TEST_F(AnimadorSpritesTest, EmpiezaQuietoMirandoHaciaAbajo) {
    EXPECT_EQ(animador.cuadro(), (Cuadro{0, 0, false, Orientacion::Abajo, 0}));
}

TEST_F(AnimadorSpritesTest, CaminandoSigueLaSecuenciaDeLaCaminata) {
    animador.actualizar(0, 1, inicio);
    EXPECT_EQ(animador.cuadro().columna, 0);
    animador.actualizar(0, 1, inicio + milliseconds(150));
    EXPECT_EQ(animador.cuadro().columna, 1);
    animador.actualizar(0, 1, inicio + milliseconds(250));
    EXPECT_EQ(animador.cuadro().columna, 0);
    animador.actualizar(0, 1, inicio + milliseconds(350));
    EXPECT_EQ(animador.cuadro().columna, 2);
}

TEST_F(AnimadorSpritesTest, DespuesDelUltimoCuadroVuelveAlPrimero) {
    animador.actualizar(0, 1, inicio);
    animador.actualizar(0, 1, inicio + milliseconds(450));
    EXPECT_EQ(animador.cuadro().columna, 0);
    animador.actualizar(0, 1, inicio + milliseconds(550));
    EXPECT_EQ(animador.cuadro().columna, 1);
}

TEST_F(AnimadorSpritesTest, CadaOrientacionUsaSuFila) {
    animador.actualizar(0, -1, inicio);
    EXPECT_EQ(animador.cuadro(), (Cuadro{0, 2, false, Orientacion::Arriba, 0}));
    animador.actualizar(1, 0, inicio);
    EXPECT_EQ(animador.cuadro(), (Cuadro{0, 1, false, Orientacion::Derecha, 0}));
}

TEST_F(AnimadorSpritesTest, HaciaLaIzquierdaEspejaElCostado) {
    animador.actualizar(-1, 0, inicio);
    EXPECT_EQ(animador.cuadro(), (Cuadro{0, 1, true, Orientacion::Izquierda, 0}));
}

TEST_F(AnimadorSpritesTest, EnDiagonalMiraDeCostado) {
    animador.actualizar(1, -1, inicio);
    EXPECT_EQ(animador.cuadro().fila, 1);
    animador.actualizar(-1, 1, inicio);
    EXPECT_TRUE(animador.cuadro().espejado);
}

TEST_F(AnimadorSpritesTest, QuietoSigueMirandoHaciaDondeIbaEnElPrimerCuadro) {
    animador.actualizar(0, -1, inicio);
    animador.actualizar(0, -1, inicio + milliseconds(150));
    animador.actualizar(0, 0, inicio + milliseconds(160));
    EXPECT_EQ(animador.cuadro(), (Cuadro{0, 2, false, Orientacion::Arriba, 0}));
}

TEST_F(AnimadorSpritesTest, AlVolverACaminarEmpiezaDesdeElPrimerCuadro) {
    animador.actualizar(1, 0, inicio);
    animador.actualizar(0, 0, inicio + milliseconds(150));
    animador.actualizar(1, 0, inicio + milliseconds(1000));
    EXPECT_EQ(animador.cuadro().columna, 0);
    animador.actualizar(1, 0, inicio + milliseconds(1100));
    EXPECT_EQ(animador.cuadro().columna, 1);
}

TEST_F(AnimadorSpritesTest, EnLosPasosBajaLaCabeza) {
    animador.actualizar(0, 1, inicio);
    EXPECT_EQ(animador.cuadro().bajada_cabeza, 0);
    animador.actualizar(0, 1, inicio + milliseconds(150));
    EXPECT_EQ(animador.cuadro().bajada_cabeza, 1);
    animador.actualizar(0, 1, inicio + milliseconds(250));
    EXPECT_EQ(animador.cuadro().bajada_cabeza, 0);
    animador.actualizar(0, 1, inicio + milliseconds(350));
    EXPECT_EQ(animador.cuadro().bajada_cabeza, 1);
}

// --- Tiras: una fila por pose, como las del esqueleto ---

TEST(FormatoTiraTest, UsaTodosLosCuadrosEnOrdenYEspejaALaIzquierda) {
    FormatoAnimacion tira = formato_tira(32, 32, 3, milliseconds(100));
    EXPECT_EQ(tira.caminata, (std::vector<int>{0, 1, 2}));
    EXPECT_EQ(tira.filas.at(Orientacion::Derecha), (std::pair<int, bool>{0, false}));
    EXPECT_EQ(tira.filas.at(Orientacion::Izquierda), (std::pair<int, bool>{0, true}));
}

TEST(FormatoTiraTest, SeAnimaAunqueEsteQuieta) {
    AnimadorSprites animador(formato_tira(32, 32, 3, milliseconds(100)));
    std::chrono::steady_clock::time_point inicio;
    animador.actualizar(0, 0, inicio);
    animador.actualizar(0, 0, inicio + milliseconds(150));
    EXPECT_EQ(animador.cuadro().columna, 1);
    animador.actualizar(0, 0, inicio + milliseconds(350));
    EXPECT_EQ(animador.cuadro().columna, 0);
}

TEST(FormatoTiraTest, SinRepetirSeQuedaEnElUltimoCuadro) {
    AnimadorSprites animador(formato_tira(32, 32, 3, milliseconds(100), false));
    std::chrono::steady_clock::time_point inicio;
    animador.actualizar(0, 0, inicio);
    animador.actualizar(0, 0, inicio + milliseconds(1000));
    EXPECT_EQ(animador.cuadro().columna, 2);
}

TEST(FormatoTiraTest, AlCambiarDeFormatoEmpiezaDesdeElPrimerCuadroMirandoIgual) {
    AnimadorSprites animador(formato_tira(32, 32, 3, milliseconds(100)));
    std::chrono::steady_clock::time_point inicio;
    animador.actualizar(-1, 0, inicio);
    animador.actualizar(-1, 0, inicio + milliseconds(150));
    animador.cambiar_formato(formato_tira(32, 32, 5, milliseconds(100)));
    animador.actualizar(0, 0, inicio + milliseconds(200));
    EXPECT_EQ(animador.cuadro().columna, 0);
    EXPECT_TRUE(animador.cuadro().espejado);
}
