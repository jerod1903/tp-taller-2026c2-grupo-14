#ifndef CARTEL_NOMBRE_H
#define CARTEL_NOMBRE_H

#include <string>

#include <SDL2pp/SDL2pp.hh>

// El nombre de un jugador ya renderizado: las letras y su sombra. Se ve sobre su personaje y en el
// mapa general.
struct CartelNombre {
    std::string texto;
    SDL2pp::Texture letras;
    SDL2pp::Texture sombra;
};

// El cartel centrado en x, con su borde de arriba en y, en píxeles de la ventana
void dibujar_cartel(SDL2pp::Renderer& renderer, CartelNombre& cartel, int centro_x, int arriba);

#endif
