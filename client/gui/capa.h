#ifndef CAPA_H
#define CAPA_H

#include <SDL2pp/SDL2pp.hh>

// Una capa del dibujo de una entidad: su hoja y cuantos cuadros a la derecha del cuerpo esta su
// cuadro (los brazos del granjero estan 6 mas alla, en la misma hoja)
struct Capa {
    SDL2pp::Texture* hoja;
    int columnas_corridas;
};

#endif
