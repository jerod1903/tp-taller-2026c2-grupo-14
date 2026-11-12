#include "mapa.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common/map/MapSprites.h"
#include "common/map/persistence/MapRepository.h"

#include "client/constants.h"
#include "rutas.h"

using stardew::maps::ELEMENT_SPRITES;
using stardew::maps::ElementType;
using stardew::maps::TILE_SPRITES;
using stardew::maps::TileType;

Mapa::Mapa(SDL2pp::Renderer& renderer, const std::string& world_id):
        hojas(cargar_hojas(renderer)),
        graficos(armar_graficos()),
        graficos_elementos(armar_graficos_elementos()),
        areas(cargar_areas(world_id)) {}

std::map<std::string, SDL2pp::Texture> Mapa::cargar_hojas(SDL2pp::Renderer& renderer) {
    std::map<std::string, SDL2pp::Texture> cargadas;
    auto cargar = [&](const char* hoja) {
        if (!cargadas.contains(hoja)) {
            cargadas.try_emplace(hoja, renderer, std::string(CARPETA_ASSETS) + hoja);
        }
    };
    for (const auto& [tipo, sprite]: TILE_SPRITES) {
        cargar(sprite.sheet);
    }
    for (const auto& [tipo, sprite]: ELEMENT_SPRITES) {
        cargar(sprite.sheet);
    }
    return cargadas;
}

std::map<TileType, TileGrafico> Mapa::armar_graficos() {
    std::map<TileType, TileGrafico> armados;
    for (const auto& [tipo, sprite]: TILE_SPRITES) {
        SDL2pp::Point casilla(sprite.column, sprite.row);
        armados.emplace(tipo, TileGrafico{hojas.at(sprite.sheet), casilla});
    }
    return armados;
}

std::map<ElementType, ElementoGrafico> Mapa::armar_graficos_elementos() {
    std::map<ElementType, ElementoGrafico> armados;
    for (const auto& [tipo, sprite]: ELEMENT_SPRITES) {
        SDL2pp::Rect origen(sprite.rect.x, sprite.rect.y, sprite.rect.width, sprite.rect.height);
        armados.emplace(tipo, ElementoGrafico{hojas.at(sprite.sheet), origen, sprite.ground});
    }
    return armados;
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
