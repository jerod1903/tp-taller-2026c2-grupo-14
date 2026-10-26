#include "caverna.h"

#include "constants.h"

Caverna::Caverna(SDL2pp::Renderer& renderer, int escala):
        tileset(renderer, RUTA_TILESET_CAVERNA), mapa(MAPA_CAVERNA), escala(escala) {}

int Caverna::tamano_tile() const { return TAMANO_TILE_ORIGINAL * escala; }

int Caverna::ancho_px() const { return static_cast<int>(mapa[0].size()) * tamano_tile(); }

int Caverna::alto_px() const { return static_cast<int>(mapa.size()) * tamano_tile(); }

SDL2pp::Point Caverna::posicion_inicial() const {
    for (size_t y = 0; y < mapa.size(); y++) {
        size_t x = mapa[y].find(INICIO);
        if (x != std::string::npos) {
            return SDL2pp::Point(static_cast<int>(x), static_cast<int>(y)) * tamano_tile();
        }
    }
    return SDL2pp::Point(tamano_tile(), tamano_tile());
}

bool Caverna::es_piso(int x, int y) const {
    if (y < 0 || y >= static_cast<int>(mapa.size()) || x < 0 ||
        x >= static_cast<int>(mapa[y].size())) {
        return false;
    }
    return mapa[y][x] != PARED;
}

void Caverna::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) {
    const int tile = tamano_tile();
    // solo se dibujan los tiles visibles por la camara
    const int x_ini = camara.x / tile;
    const int y_ini = camara.y / tile;
    const int x_fin = (camara.x + camara.w) / tile;
    const int y_fin = (camara.y + camara.h) / tile;

    for (int y = y_ini; y <= y_fin && y < static_cast<int>(mapa.size()); y++) {
        for (int x = x_ini; x <= x_fin && x < static_cast<int>(mapa[y].size()); x++) {
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
