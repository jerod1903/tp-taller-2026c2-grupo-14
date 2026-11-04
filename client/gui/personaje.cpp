#include "personaje.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "client/constants.h"

Personaje::Personaje(SDL2pp::Texture& textura): textura(textura) {}

void Personaje::actualizar(const PlayerState& estado, uint32_t tick,
                           std::chrono::steady_clock::time_point ahora) {
    if (!this->tick || *this->tick != tick) {
        // El servidor manda el centro de la caja de colision en milesimas de casilla; los pies van
        // en su borde de abajo
        SDL2pp::Point nuevo(
                static_cast<int>(estado.x * TAMANO_TILE / POSITION_SCALE),
                static_cast<int>(estado.y * TAMANO_TILE / POSITION_SCALE) +
                        static_cast<int>(CHARACTER_HALF_HEIGHT * TAMANO_TILE));
        // Al aparecer o al pasar por un portal salta, no camina hasta ahi
        bool salta = !this->tick || estado.areaId != area_id;
        desde = salta ? nuevo : pies;
        hasta = nuevo;
        llegada = ahora;
        this->tick = tick;
        area_id = estado.areaId;
    }
    float avance = std::min(1.0f, (ahora - llegada) / DURACION_TICK_SERVIDOR);
    pies = SDL2pp::Point(desde.x + static_cast<int>(std::lround((hasta.x - desde.x) * avance)),
                         desde.y + static_cast<int>(std::lround((hasta.y - desde.y) * avance)));
}

Sprite Personaje::sprite() const {
    // El sprite esta centrado en x y los pies son su ultima fila
    return Sprite{&textura, FRAME,
                  SDL2pp::Rect(pies.x - FRAME.w / 2, pies.y - FRAME.h, FRAME.w, FRAME.h)};
}

SDL2pp::Point Personaje::centro() const { return SDL2pp::Point(pies.x, pies.y - FRAME.h / 2); }

const std::string& Personaje::area() const { return area_id; }
