#ifndef HERRAMIENTA_EN_MANO_H
#define HERRAMIENTA_EN_MANO_H

#include <optional>

#include "client/gui/sprites/animador_sprites.h"
#include "common/farm_dto.h"

struct HerramientaEnMano {
    int dx;
    int dy;
    bool espejada;
    bool detras;
};

std::optional<HerramientaEnMano> herramienta_en_mano_de(ItemType item, Orientacion mirando,
                                                        int ancho_icono);

#endif
