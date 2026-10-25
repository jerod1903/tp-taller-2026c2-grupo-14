#include "calculador_camara.h"

CalculadorCamara::CalculadorCamara(int ancho_vista, int alto_vista):
        ancho_vista(ancho_vista), alto_vista(alto_vista) {}

int CalculadorCamara::limitar(int valor, int maximo) const {
    maximo = SDL_max(0, maximo); // por si el mapa es mas chico q la ventana
    return SDL_max(0, SDL_min(valor, maximo));
}

SDL2pp::Rect CalculadorCamara::calcular(const SDL2pp::Point& centro, int ancho_mapa,
                                        int alto_mapa) const {
    int x = limitar(centro.x - ancho_vista / 2, ancho_mapa - ancho_vista);
    int y = limitar(centro.y - alto_vista / 2, alto_mapa - alto_vista);
    return SDL2pp::Rect(x, y, ancho_vista, alto_vista);
}
