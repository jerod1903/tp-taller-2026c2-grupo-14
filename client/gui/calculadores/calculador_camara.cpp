#include "client/gui/calculadores/calculador_camara.h"

#include <SDL2/SDL.h>

void CalculadorCamara::ajustar_vista(int ancho, int alto) {
    ancho_vista = ancho;
    alto_vista = alto;
}

int CalculadorCamara::limitar(int valor, int maximo) const {
    maximo = SDL_max(0, maximo);
    return SDL_max(0, SDL_min(valor, maximo));
}

SDL2pp::Rect CalculadorCamara::calcular(const SDL2pp::Point& centro, int ancho_mapa,
                                        int alto_mapa) const {
    int x = limitar(centro.x - ancho_vista / 2, ancho_mapa - ancho_vista);
    int y = limitar(centro.y - alto_vista / 2, alto_mapa - alto_vista);
    return SDL2pp::Rect(x, y, ancho_vista, alto_vista);
}
