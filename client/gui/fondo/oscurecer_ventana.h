#ifndef OSCURECER_VENTANA_H
#define OSCURECER_VENTANA_H

#include <SDL2pp/SDL2pp.hh>

// Tapa toda la ventana con COLOR_FONDO_PAUSA, semitransparente: el fondo de lo que se abre encima
// del juego (la pausa, el fin del día, el mapa general)
void oscurecer_ventana(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana);

#endif
