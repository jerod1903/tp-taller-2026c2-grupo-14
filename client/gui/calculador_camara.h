#ifndef CALCULADOR_CAMARA_H
#define CALCULADOR_CAMARA_H

#include <SDL2pp/SDL2pp.hh>

class CalculadorCamara {
public:
    // Lo que entra en la pantalla, en pixeles del mundo. Cambia al redimensionar la ventana
    void ajustar_vista(int ancho, int alto);
    SDL2pp::Rect calcular(const SDL2pp::Point& centro, int ancho_mapa, int alto_mapa) const;

private:
    int limitar(int valor, int maximo) const;  // entre 0 y maximo
    int ancho_vista = 0;
    int alto_vista = 0;
};

#endif
