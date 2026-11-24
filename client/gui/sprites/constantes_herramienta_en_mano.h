#ifndef CONSTANTES_HERRAMIENTA_EN_MANO_H
#define CONSTANTES_HERRAMIENTA_EN_MANO_H

#include <map>
#include <tuple>
#include <utility>

#include "client/gui/sprites/animador_sprites.h"
#include "common/farm_dto.h"

inline const std::map<ItemType, std::pair<int, int>> EMPUNADURAS = {
        {ItemType::Pickaxe, {2, 10}},
        {ItemType::WateringCan, {1, 1}},
        {ItemType::Shovel, {2, 10}},
        {ItemType::Sword, {3, 11}},
};
inline const std::map<Orientacion, std::tuple<int, int, bool, bool>> POSICIONES_EN_MANO = {
        {Orientacion::Abajo, {5, -11, false, false}},
        {Orientacion::Derecha, {2, -11, false, false}},
        {Orientacion::Izquierda, {-2, -11, true, false}},
        {Orientacion::Arriba, {-5, -11, true, true}},
};

#endif
