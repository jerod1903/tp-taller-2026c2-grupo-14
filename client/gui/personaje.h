#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <SDL2pp/SDL2pp.hh>

// Un jugador en pantalla. Solo dibuja: la posicion la decide el servidor.
class Personaje {
public:
    // sprite: compartido entre todos los personajes, se carga una sola vez
    Personaje(SDL2pp::Texture& sprite, int escala);
    // posicion: esquina superior izquierda del sprite, en pixeles del tileset (sin escalar)
    void actualizar(const SDL2pp::Point& nueva_posicion);
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) const;
    // En pixeles de pantalla (ya escalado), para la camara
    SDL2pp::Point centro() const;

private:
    SDL2pp::Texture& sprite;
    SDL2pp::Point posicion;
    int escala;
};

#endif
