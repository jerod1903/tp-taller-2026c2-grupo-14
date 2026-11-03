#include "personaje.h"

#include <string>

#include "constants.h"

Personaje::Personaje(SDL2pp::Texture& sprite, int escala): sprite(sprite), escala(escala) {}

void Personaje::actualizar(const PlayerState& estado) {
    // El servidor manda la casilla y cuanto avanzo hacia la vecina; se pasa a pixeles del tileset
    int x = estado.x * TAMANO_TILE + estado.dirX * estado.progress * TAMANO_TILE / PROGRESS_SCALE;
    int y = estado.y * TAMANO_TILE + estado.dirY * estado.progress * TAMANO_TILE / PROGRESS_SCALE;
    // El sprite es mas alto que una casilla: los pies quedan sobre la casilla
    posicion = SDL2pp::Point(x, y - (FRAME.h - TAMANO_TILE));
    area_id = estado.areaId;
}

SDL2pp::Point Personaje::centro() const {
    return (posicion + SDL2pp::Point(FRAME.w, FRAME.h) / 2) * escala;
}

const std::string& Personaje::area() const { return area_id; }

void Personaje::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) const {
    SDL2pp::Rect destino(posicion.x * escala - camara.x, posicion.y * escala - camara.y,
                         FRAME.w * escala, FRAME.h * escala);
    renderer.Copy(sprite, FRAME, destino);
}
