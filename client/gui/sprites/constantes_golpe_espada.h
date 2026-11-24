#ifndef CONSTANTES_GOLPE_ESPADA_H
#define CONSTANTES_GOLPE_ESPADA_H

#include <map>
#include <tuple>

#include "client/gui/sprites/animador_sprites.h"

inline constexpr double BARRIDO_GOLPE_ESPADA = 120.0;
inline const std::map<Orientacion, std::tuple<double, int, int>> POSICIONES_GOLPE_ESPADA = {
        {Orientacion::Derecha, {45.0, 5, -8}},
        {Orientacion::Izquierda, {-135.0, -5, -8}},
        {Orientacion::Abajo, {135.0, 0, -6}},
        {Orientacion::Arriba, {-45.0, 0, -12}},
};

#endif
