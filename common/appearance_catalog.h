#ifndef APPEARANCE_CATALOG_H
#define APPEARANCE_CATALOG_H

#include <cstdint>

#include "common/lobby_dto.h"

// Cantidad de opciones de cada parte del personaje. Cada id de CharacterAppearance es un índice
// en [0, cantidad). Lo usan el servidor para validar y el cliente para armar los selectores.
// TODO: confirmar las cantidades con los sprites; solo los colores salen de la paleta del cliente.
namespace AppearanceCatalog {

constexpr uint16_t HAIR_STYLES = 8;
constexpr uint8_t HAIR_COLORS = 16;
constexpr uint16_t SHIRTS = 8;
constexpr uint16_t PANTS = 8;
constexpr uint16_t HATS = 5;  // incluye el 0 = sin sombrero

constexpr bool is_valid(const CharacterAppearance& appearance) {
    return appearance.hairStyle < HAIR_STYLES && appearance.hairColor < HAIR_COLORS &&
           appearance.shirt < SHIRTS && appearance.pants < PANTS && appearance.hat < HATS;
}

}  // namespace AppearanceCatalog

#endif  // APPEARANCE_CATALOG_H
