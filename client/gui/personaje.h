#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <SDL2pp/SDL2pp.hh>

#include "caverna.h"

class Personaje {
public:
    Personaje(SDL2pp::Renderer& renderer, int escala, const SDL2pp::Point& posicion);
    void mover(int dx, int dy, const Caverna& caverna);
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara);
    SDL2pp::Point centro() const;

private:
    SDL2pp::Rect hitbox(const SDL2pp::Point& pos) const;
    SDL2pp::Texture sprite;
    SDL2pp::Point posicion;
    int escala;
};

#endif
