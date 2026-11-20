#include "client/mapa-general/grilla_mapa.h"

#include <algorithm>
#include <climits>
#include <vector>

#include "client/mapa-general/colores_mapa.h"

namespace maps = stardew::maps;

namespace {

// Las del mundo: la granja, el pueblo y los pisos de la mina. Los interiores van aparte.
bool es_del_mundo(const maps::MapArea& area) {
    return area.type == maps::AreaType::Farm || area.type == maps::AreaType::Village ||
           area.type == maps::AreaType::DungeonLevel;
}

const ColorMapa& color_terreno(maps::TileType tipo) {
    auto color = COLORES_TERRENO.find(tipo);
    return color != COLORES_TERRENO.end() ? color->second : COLOR_DESCONOCIDO;
}

// Ubica cada área del mundo, corrida para que la de más arriba y la de más a la izquierda
// arranquen en 0
std::map<std::string, LugarArea> ubicar_areas(const maps::MapDefinition& mapa, int& ancho,
                                              int& alto) {
    int min_x = INT_MAX;
    int min_y = INT_MAX;
    int max_x = INT_MIN;
    int max_y = INT_MIN;
    for (const maps::MapArea& area: mapa.areas) {
        if (es_del_mundo(area)) {
            min_x = std::min(min_x, area.offsetX);
            min_y = std::min(min_y, area.offsetY);
            max_x = std::max(max_x, area.offsetX + area.width);
            max_y = std::max(max_y, area.offsetY + area.height);
        }
    }
    std::map<std::string, LugarArea> lugares;
    if (min_x == INT_MAX) {
        ancho = 0;
        alto = 0;
        return lugares;
    }
    ancho = max_x - min_x;
    alto = max_y - min_y;
    for (const maps::MapArea& area: mapa.areas) {
        if (es_del_mundo(area)) {
            lugares[area.id] =
                    LugarArea{area.offsetX - min_x, area.offsetY - min_y, area.width, area.height};
        }
    }
    return lugares;
}

void pintar(GrillaMapa& grilla, int x, int y, const ColorMapa& color) {
    if (x >= 0 && y >= 0 && x < grilla.ancho && y < grilla.alto) {
        grilla.colores[y * grilla.ancho + x] = color;
    }
}

}  // namespace

GrillaMapa grilla_del_mapa(const maps::MapDefinition& mapa) {
    GrillaMapa grilla;
    grilla.areas = ubicar_areas(mapa, grilla.ancho, grilla.alto);
    grilla.colores.assign(static_cast<std::size_t>(grilla.ancho) * grilla.alto, COLOR_FONDO);
    for (const maps::MapArea& area: mapa.areas) {
        auto lugar = grilla.areas.find(area.id);
        if (lugar == grilla.areas.end()) {
            continue;
        }
        // Una grilla más chica que lo declarado (mapa con errores) pinta lo que tiene
        for (int y = 0; y < std::min<int>(area.height, area.tiles.size()); ++y) {
            for (int x = 0; x < std::min<int>(area.width, area.tiles[y].size()); ++x) {
                pintar(grilla, lugar->second.x + x, lugar->second.y + y,
                       color_terreno(area.tiles[y][x]));
            }
        }
    }
    for (const maps::MapElement& elemento: mapa.elements) {
        auto lugar = grilla.areas.find(elemento.areaId);
        auto color = elemento.type ? COLORES_ELEMENTO.find(*elemento.type) : COLORES_ELEMENTO.end();
        if (lugar == grilla.areas.end() || color == COLORES_ELEMENTO.end()) {
            continue;
        }
        for (int y = elemento.y; y < elemento.y + elemento.height; ++y) {
            for (int x = elemento.x; x < elemento.x + elemento.width; ++x) {
                pintar(grilla, lugar->second.x + x, lugar->second.y + y, color->second);
            }
        }
    }
    return grilla;
}
