#ifndef CAVERNA_H
#define CAVERNA_H

#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "common/map/MapDefinition.h"

class Caverna {
public:
    Caverna(SDL2pp::Renderer& renderer, int escala, const std::string& ruta_mapa);
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara);
    bool choca(const SDL2pp::Rect& area) const;
    int ancho_px() const;
    int alto_px() const;
    int tamano_tile() const;
    SDL2pp::Point posicion_inicial() const;

private:
    bool es_piso(int x, int y) const;
    SDL2pp::Texture tileset;
    std::vector<std::vector<stardew::maps::TileType>> tiles;
    SDL2pp::Point inicio;  // en tiles
    int escala;
};

#endif
