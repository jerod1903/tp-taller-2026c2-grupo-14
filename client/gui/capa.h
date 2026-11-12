#ifndef CAPA_H
#define CAPA_H

#include <map>
#include <utility>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "animador_sprites.h"

// Como se elige el recorte de una capa dentro de su hoja
enum class TipoCapa {
    PorCuadro,  // cuerpo, pantalon, brazos: el mismo cuadro de la caminata que el cuerpo
    PorVista,   // camisa, pelo, sombrero: una sola imagen por orientacion, que sigue a la cabeza
};

// orientacion -> (numero de vista en la hoja, si se espeja)
using Vistas = std::map<Orientacion, std::pair<int, bool>>;

// El tinte que deja la imagen como esta
inline constexpr SDL_Color SIN_TINTE = {255, 255, 255, 255};

// Una capa del dibujo de una entidad. Las capas se dibujan una encima de la otra, en orden
struct Capa {
    SDL2pp::Texture* hoja;
    TipoCapa tipo;
    SDL2pp::Point celda;           // esquina de esta prenda dentro de su hoja
    SDL2pp::Point tamano;          // del recorte
    SDL2pp::Point desplazamiento;  // respecto de la esquina del cuadro del cuerpo
    // Solo PorVista: orientacion -> (numero de vista, si se espeja). Las vistas van una debajo de
    // la otra, cada una de tamano.y de alto
    Vistas vistas = {};
    SDL_Color tinte = SIN_TINTE;  // el pelo y el pantalon vienen en gris
};

#endif
