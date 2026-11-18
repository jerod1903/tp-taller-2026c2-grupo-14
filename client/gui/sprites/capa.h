#ifndef CAPA_H
#define CAPA_H

#include <map>
#include <utility>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/gui/sprites/animador_sprites.h"

enum class TipoCapa {
    PorCuadro,
    PorVista,
};

using Vistas = std::map<Orientacion, std::pair<int, bool>>;

inline constexpr SDL_Color SIN_TINTE = {255, 255, 255, 255};

struct Capa {
    SDL2pp::Texture* hoja;
    TipoCapa tipo;
    SDL2pp::Point celda;
    SDL2pp::Point tamano;
    SDL2pp::Point desplazamiento;
    Vistas vistas = {};
    SDL_Color tinte = SIN_TINTE;
};

// Cómo se dibuja una entidad: el formato de su hoja y las capas que la forman.
struct Aspecto {
    const FormatoAnimacion* formato;
    std::vector<Capa> capas;
};

#endif
