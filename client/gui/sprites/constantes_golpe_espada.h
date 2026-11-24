#ifndef CONSTANTES_GOLPE_ESPADA_H
#define CONSTANTES_GOLPE_ESPADA_H

#include <map>
#include <tuple>

#include "client/gui/sprites/animador_sprites.h"

inline constexpr double BARRIDO_GOLPE_ESPADA = 120.0;
inline constexpr int EMPUNADURA_ESPADA_X = 3;
inline constexpr int EMPUNADURA_ESPADA_Y = 11;
inline const std::map<Orientacion, std::tuple<double, int, int>> POSICIONES_GOLPE_ESPADA = {
        {Orientacion::Derecha, {45.0, 3, -12}},
        {Orientacion::Izquierda, {-135.0, -3, -12}},
        {Orientacion::Abajo, {135.0, 3, -11}},
        {Orientacion::Arriba, {-45.0, -3, -13}},
};

#endif
