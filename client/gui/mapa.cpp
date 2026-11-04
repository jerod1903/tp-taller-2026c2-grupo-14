#include "mapa.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common/map/persistence/MapRepository.h"

#include "client/constants.h"
#include "rutas.h"

using stardew::maps::ElementType;
using stardew::maps::TileType;

Mapa::Mapa(SDL2pp::Renderer& renderer, const std::string& world_id):
        tileset_caverna(renderer, RUTA_TILESET_CAVERNA),
        tileset_exterior(renderer, RUTA_TILESET_EXTERIOR),
        cabanas(renderer, RUTA_CABANAS),
        edificios_pueblo(renderer, RUTA_EDIFICIOS_PUEBLO),
        npc_marnie(renderer, RUTA_NPC_MARNIE),
        npc_clint(renderer, RUTA_NPC_CLINT),
        npc_pierre(renderer, RUTA_NPC_PIERRE),
        npc_robin(renderer, RUTA_NPC_ROBIN),
        graficos(armar_graficos()),
        graficos_elementos(armar_graficos_elementos()),
        areas(cargar_areas(world_id)) {}

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

std::map<ElementType, ElementoGrafico> Mapa::armar_graficos_elementos() {
    // La entrada a la mina no tiene dibujo propio: se pinta el piso de la caverna
    SDL2pp::Rect piso_caverna(TILE_PISO.x * TAMANO_TILE, TILE_PISO.y * TAMANO_TILE, TAMANO_TILE,
                              TAMANO_TILE);
    return {
            {ElementType::Cabin, {cabanas, RECORTE_CABANA, false}},
            {ElementType::Market, {edificios_pueblo, RECORTE_ALMACEN, false}},
            {ElementType::CommunityCenter, {edificios_pueblo, RECORTE_CENTRO_COMUNITARIO, false}},
            {ElementType::DungeonEntrance, {tileset_caverna, piso_caverna, true}},
            {ElementType::MerchantMarnie, {npc_marnie, FRAME_NPC, false}},
            {ElementType::MerchantClint, {npc_clint, FRAME_NPC, false}},
            {ElementType::MerchantPierre, {npc_pierre, FRAME_NPC, false}},
            {ElementType::MerchantRobin, {npc_robin, FRAME_NPC, false}},
    };
}

std::map<std::string, Mapa::Area> Mapa::cargar_areas(const std::string& world_id) const {
    // El id viene del servidor: se valida para que no pueda apuntar a un archivo fuera de maps/
    if (world_id.empty() || world_id.find_first_not_of(CARACTERES_WORLD_ID) != std::string::npos) {
        throw std::runtime_error("world id invalido: " + world_id);
    }
    stardew::maps::TomlMapRepository repositorio;
    stardew::maps::MapDefinition definicion =
            repositorio.loadMap(std::string(TALLER_MAPS_DIR) + "/" + world_id + ".toml");
    std::map<std::string, Area> cargadas;
    for (stardew::maps::MapArea& area: definicion.areas) {
        cargadas[area.id].tiles = std::move(area.tiles);
    }
    for (const stardew::maps::MapElement& elemento: definicion.elements) {
        auto area = cargadas.find(elemento.areaId);
        auto grafico = elemento.type ? graficos_elementos.find(*elemento.type)
                                     : graficos_elementos.end();
        if (area == cargadas.end() || grafico == graficos_elementos.end()) {
            continue;  // los que no se ven (spawn, puertas) o todavia no tienen dibujo
        }
        const SDL2pp::Rect& origen = grafico->second.origen;
        int ancho = elemento.width * TAMANO_TILE;
        int alto = origen.h * ancho / origen.w;
        SDL2pp::Rect destino(elemento.x * TAMANO_TILE,
                             (elemento.y + elemento.height) * TAMANO_TILE - alto, ancho, alto);
        Sprite sprite{&grafico->second.textura, origen, destino};
        if (grafico->second.es_suelo) {
            area->second.suelo.push_back(sprite);
        } else {
            area->second.elementos.push_back(sprite);
        }
    }
    return cargadas;
}

void Mapa::mostrar_area(const std::string& area_id) {
    auto area = areas.find(area_id);
    area_actual = area == areas.end() ? nullptr : &area->second;
}

void Mapa::dibujar_suelo(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) {
    if (!area_actual) {
        return;
    }
    const Tiles& tiles = area_actual->tiles;
    // Solo las casillas que entran en la camara
    int primera_fila = std::max(0, camara.y / TAMANO_TILE);
    int ultima_fila = std::min(static_cast<int>(tiles.size()) - 1,
                               (camara.y + camara.h - 1) / TAMANO_TILE);
    for (int fila = primera_fila; fila <= ultima_fila; ++fila) {
        int primera_columna = std::max(0, camara.x / TAMANO_TILE);
        int ultima_columna = std::min(static_cast<int>(tiles[fila].size()) - 1,
                                      (camara.x + camara.w - 1) / TAMANO_TILE);
        for (int columna = primera_columna; columna <= ultima_columna; ++columna) {
            auto grafico = graficos.find(tiles[fila][columna]);
            if (grafico == graficos.end()) {
                continue;  // todavia no hay grafico para ese tipo
            }
            const SDL2pp::Point& casilla = grafico->second.casilla;
            SDL2pp::Rect origen(casilla.x * TAMANO_TILE, casilla.y * TAMANO_TILE, TAMANO_TILE,
                                TAMANO_TILE);
            SDL2pp::Rect destino(columna * TAMANO_TILE - camara.x, fila * TAMANO_TILE - camara.y,
                                 TAMANO_TILE, TAMANO_TILE);
            renderer.Copy(grafico->second.textura, origen, destino);
        }
    }
    for (const Sprite& sprite: area_actual->suelo) {
        sprite.dibujar(renderer, camara);
    }
}

const std::vector<Sprite>& Mapa::elementos() const {
    static const std::vector<Sprite> ninguno;
    return area_actual ? area_actual->elementos : ninguno;
}

int Mapa::ancho_px() const {
    if (!area_actual || area_actual->tiles.empty()) {
        return 0;
    }
    return static_cast<int>(area_actual->tiles.front().size()) * TAMANO_TILE;
}

int Mapa::alto_px() const {
    return area_actual ? static_cast<int>(area_actual->tiles.size()) * TAMANO_TILE : 0;
}
