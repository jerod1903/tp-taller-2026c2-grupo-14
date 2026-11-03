#include "mapa.h"

#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common/map/persistence/MapRepository.h"

#include "constants.h"

using stardew::maps::TileType;

Mapa::Mapa(SDL2pp::Renderer& renderer, const std::string& world_id, int escala):
        tileset_caverna(renderer, RUTA_TILESET_CAVERNA),
        tileset_exterior(renderer, RUTA_TILESET_EXTERIOR),
        graficos(armar_graficos()),
        areas(cargar_areas(world_id)),
        escala(escala) {}

std::map<TileType, TileGrafico> Mapa::armar_graficos() {
    return {
            {TileType::CaveWall, {tileset_caverna, TILE_PARED}},
            {TileType::CaveFloor, {tileset_caverna, TILE_PISO}},
            {TileType::Grass, {tileset_exterior, TILE_PASTO}},
            {TileType::Dirt, {tileset_exterior, TILE_TIERRA}},
            {TileType::Path, {tileset_exterior, TILE_CAMINO}},
            {TileType::TilledSoil, {tileset_exterior, TILE_TIERRA_ARADA}},
            {TileType::Water, {tileset_exterior, TILE_AGUA}},
            {TileType::Sand, {tileset_exterior, TILE_ARENA}},
            {TileType::Stone, {tileset_exterior, TILE_PIEDRA}},
            {TileType::WoodFloor, {tileset_exterior, TILE_PISO_MADERA}},
    };
}

std::map<std::string, Mapa::Tiles> Mapa::cargar_areas(const std::string& world_id) {
    // El id viene del servidor: se valida para que no pueda apuntar a un archivo fuera de maps/
    if (world_id.empty() || world_id.find_first_not_of(CARACTERES_WORLD_ID) != std::string::npos) {
        throw std::runtime_error("world id invalido: " + world_id);
    }
    stardew::maps::TomlMapRepository repositorio;
    stardew::maps::MapDefinition definicion =
            repositorio.loadMap(std::string(TALLER_MAPS_DIR) + "/" + world_id + ".toml");
    std::map<std::string, Tiles> areas;
    for (stardew::maps::MapArea& area: definicion.areas) {
        areas.emplace(area.id, std::move(area.tiles));
    }
    return areas;
}

void Mapa::mostrar_area(const std::string& area_id) {
    auto area = areas.find(area_id);
    area_actual = area == areas.end() ? nullptr : &area->second;
}

void Mapa::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) {
    if (!area_actual) {
        return;
    }
    const Tiles& tiles = *area_actual;
    int tamano = TAMANO_TILE * escala;
    for (size_t fila = 0; fila < tiles.size(); ++fila) {
        for (size_t columna = 0; columna < tiles[fila].size(); ++columna) {
            auto grafico = graficos.find(tiles[fila][columna]);
            if (grafico == graficos.end()) {
                continue;  // todavia no hay grafico para ese tipo
            }
            const SDL2pp::Point& casilla = grafico->second.casilla;
            SDL2pp::Rect origen(casilla.x * TAMANO_TILE, casilla.y * TAMANO_TILE, TAMANO_TILE,
                                TAMANO_TILE);
            SDL2pp::Rect destino(static_cast<int>(columna) * tamano - camara.x,
                                 static_cast<int>(fila) * tamano - camara.y, tamano, tamano);
            renderer.Copy(grafico->second.textura, origen, destino);
        }
    }
}

int Mapa::ancho_px() const {
    if (!area_actual || area_actual->empty()) {
        return 0;
    }
    return static_cast<int>(area_actual->front().size()) * TAMANO_TILE * escala;
}

int Mapa::alto_px() const {
    return area_actual ? static_cast<int>(area_actual->size()) * TAMANO_TILE * escala : 0;
}
