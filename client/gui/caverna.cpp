#include "caverna.h"

#include <algorithm>
#include <stdexcept>

#include "common/map/persistence/TomlMapFile.h"

#include "constants.h"

namespace maps = stardew::maps;

namespace {

// Por ahora la caverna es la primera area del mapa
const maps::MapArea& primera_area(const maps::MapDefinition& mapa) {
    if (mapa.areas.empty() || mapa.areas.front().tiles.empty()) {
        throw std::runtime_error("el mapa '" + mapa.name + "' no tiene areas con tiles");
    }
    return mapa.areas.front();
}

// El spawn_point del area marca donde arranca el personaje
SDL2pp::Point buscar_inicio(const maps::MapDefinition& mapa, const std::string& area_id) {
    const auto spawn = std::find_if(
            mapa.elements.begin(), mapa.elements.end(), [&area_id](const auto& elemento) {
                return elemento.areaId == area_id && elemento.type == maps::ElementType::SpawnPoint;
            });
    if (spawn == mapa.elements.end()) {
        return SDL2pp::Point(1, 1);
    }
    return SDL2pp::Point(spawn->x, spawn->y);
}

}  // namespace

Caverna::Caverna(SDL2pp::Renderer& renderer, int escala, const std::string& ruta_mapa):
        tileset(renderer, RUTA_TILESET_CAVERNA), escala(escala) {
    const maps::MapDefinition mapa = maps::loadTomlMapDefinition(ruta_mapa);
    const maps::MapArea& area = primera_area(mapa);
    tiles = area.tiles;
    inicio = buscar_inicio(mapa, area.id);
}

int Caverna::tamano_tile() const { return TAMANO_TILE_ORIGINAL * escala; }

int Caverna::ancho_px() const { return static_cast<int>(tiles[0].size()) * tamano_tile(); }

int Caverna::alto_px() const { return static_cast<int>(tiles.size()) * tamano_tile(); }

SDL2pp::Point Caverna::posicion_inicial() const { return inicio * tamano_tile(); }

bool Caverna::es_piso(int x, int y) const {
    if (y < 0 || y >= static_cast<int>(tiles.size()) || x < 0 ||
        x >= static_cast<int>(tiles[y].size())) {
        return false;
    }
    // por ahora solo cave_wall bloquea el paso
    return tiles[y][x] != maps::TileType::CaveWall;
}

void Caverna::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) {
    const int tile = tamano_tile();
    // solo se dibujan los tiles visibles por la camara
    const int x_ini = camara.x / tile;
    const int y_ini = camara.y / tile;
    const int x_fin = (camara.x + camara.w) / tile;
    const int y_fin = (camara.y + camara.h) / tile;

    for (int y = y_ini; y <= y_fin && y < static_cast<int>(tiles.size()); y++) {
        for (int x = x_ini; x <= x_fin && x < static_cast<int>(tiles[y].size()); x++) {
            SDL2pp::Point tile_tileset = es_piso(x, y) ? TILE_PISO : TILE_PARED;
            SDL2pp::Point origen = tile_tileset * TAMANO_TILE_ORIGINAL;
            SDL2pp::Rect fuente(origen.x, origen.y, TAMANO_TILE_ORIGINAL, TAMANO_TILE_ORIGINAL);
            SDL2pp::Rect destino(x * tile - camara.x, y * tile - camara.y, tile, tile);
            renderer.Copy(tileset, fuente, destino);
        }
    }
}

bool Caverna::choca(const SDL2pp::Rect& area) const {
    const int tile = tamano_tile();
    for (int y = area.y / tile; y <= area.GetY2() / tile; y++) {
        for (int x = area.x / tile; x <= area.GetX2() / tile; x++) {
            if (!es_piso(x, y)) {
                return true;
            }
        }
    }
    return false;
}
