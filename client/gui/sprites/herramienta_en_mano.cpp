#include "client/gui/sprites/herramienta_en_mano.h"

#include "client/gui/sprites/constantes_herramienta_en_mano.h"

std::optional<HerramientaEnMano> herramienta_en_mano_de(ItemType item, Orientacion mirando,
                                                        int ancho_icono) {
    auto empunadura = EMPUNADURAS.find(item);
    if (empunadura == EMPUNADURAS.end()) {
        return std::nullopt;
    }
    const auto& [mano_x, mano_y, espejada, detras] = POSICIONES_EN_MANO.at(mirando);
    auto [agarre_x, agarre_y] = empunadura->second;
    if (espejada) {
        agarre_x = ancho_icono - 1 - agarre_x;
    }
    return HerramientaEnMano{mano_x - agarre_x, mano_y - agarre_y, espejada, detras};
}
