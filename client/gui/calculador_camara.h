#ifndef CALCULADOR_CAMARA_H
#define CALCULADOR_CAMARA_H

#include <SDL2pp/SDL2pp.hh>

class CalculadorCamara {
public:
    CalculadorCamara(int ancho_vista, int alto_vista);
    SDL2pp::Rect calcular(const SDL2pp::Point& centro, int ancho_mapa, int alto_mapa) const;

private:
    int limitar(int valor, int maximo) const; // entre 0 y maximo
    int ancho_vista;
    int alto_vista;
};

#endif
