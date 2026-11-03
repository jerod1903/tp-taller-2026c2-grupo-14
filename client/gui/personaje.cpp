#include "personaje.h"

#include "constants.h"

Personaje::Personaje(SDL2pp::Texture& sprite, int escala): sprite(sprite), escala(escala) {}

void Personaje::actualizar(const SDL2pp::Point& nueva_posicion) { posicion = nueva_posicion; }

SDL2pp::Point Personaje::centro() const {
    return (posicion + SDL2pp::Point(FRAME.w, FRAME.h) / 2) * escala;
}

void Personaje::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) const {
    SDL2pp::Rect destino(posicion.x * escala - camara.x, posicion.y * escala - camara.y,
                         FRAME.w * escala, FRAME.h * escala);
    renderer.Copy(sprite, FRAME, destino);
}
