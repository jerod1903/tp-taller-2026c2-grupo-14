#include "personaje.h"

#include "constants.h"

Personaje::Personaje(SDL2pp::Renderer& renderer, int escala, const SDL2pp::Point& posicion):
        sprite(renderer, RUTA_SPRITE_PERSONAJE), posicion(posicion), escala(escala) {}

SDL2pp::Rect Personaje::hitbox(const SDL2pp::Point& pos) const {
    return SDL2pp::Rect(pos.x + (PIES.x * escala), pos.y + (PIES.y * escala), PIES.w * escala,
                        PIES.h * escala);
}

void Personaje::mover(int dx, int dy, const Caverna& caverna) {
    SDL2pp::Point nueva(posicion.x + dx, posicion.y);
    if (!caverna.choca(hitbox(nueva))) {
        posicion = nueva;
    }
    nueva = SDL2pp::Point(posicion.x, posicion.y + dy);
    if (!caverna.choca(hitbox(nueva))) {
        posicion = nueva;
    }
}

SDL2pp::Point Personaje::centro() const {
    return posicion + SDL2pp::Point(FRAME.w, FRAME.h) * escala / 2;
}

void Personaje::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) {
    SDL2pp::Rect destino(posicion.x - camara.x, posicion.y - camara.y, FRAME.w * escala,
                         FRAME.h * escala);
    renderer.Copy(sprite, FRAME, destino);
}
