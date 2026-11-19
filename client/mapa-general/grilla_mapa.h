#ifndef GRILLA_MAPA_H
#define GRILLA_MAPA_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "common/map/MapDefinition.h"

// El mapa general como lo muestra el editor (cada área en su offset), en colores planos: una
// casilla, un color. Sin SDL ni Qt: con esto dibujan el mapa grande y el del HUD (SDL) y la vista
// previa del menú (Qt).

struct ColorMapa {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    bool operator==(const ColorMapa&) const = default;
};

// Dónde queda un área en la grilla, en casillas
struct LugarArea {
    int x;
    int y;
    int ancho;
    int alto;

    bool operator==(const LugarArea&) const = default;
};

struct GrillaMapa {
    int ancho = 0;
    int alto = 0;
    // Por filas: la casilla (x, y) está en y * ancho + x. Lo que no es de ningún área, COLOR_FONDO.
    std::vector<ColorMapa> colores;
    // Las áreas del mundo por id, para ubicar a los jugadores. Los interiores no están.
    std::map<std::string, LugarArea> areas;

    const ColorMapa& color(int x, int y) const { return colores[y * ancho + x]; }
};

// El terreno de cada casilla y encima la base de cada elemento. La grilla arranca en el área de
// más arriba y la de más a la izquierda: los offsets negativos o lejos de cero no dejan márgenes.
GrillaMapa grilla_del_mapa(const stardew::maps::MapDefinition& mapa);

#endif
