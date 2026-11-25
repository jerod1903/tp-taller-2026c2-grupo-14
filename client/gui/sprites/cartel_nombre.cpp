#include "client/gui/sprites/cartel_nombre.h"

void dibujar_cartel(SDL2pp::Renderer& renderer, CartelNombre& cartel, int centro_x, int arriba) {
    SDL2pp::Point tamano = cartel.letras.GetSize();
    SDL2pp::Rect destino(centro_x - tamano.x / 2, arriba, tamano.x, tamano.y);
    renderer.Copy(cartel.sombra, SDL2pp::NullOpt, destino + SDL2pp::Point(1, 1));
    renderer.Copy(cartel.letras, SDL2pp::NullOpt, destino);
}
