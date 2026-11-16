#include "calculador_posicion.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "client/constants.h"
#include "common/game_dto.h"

SDL2pp::Point CalculadorPosicion::pies_desde_servidor(uint32_t x, uint32_t y) {
    return SDL2pp::Point(static_cast<int>(x * TAMANO_TILE / POSITION_SCALE),
                         static_cast<int>(y * TAMANO_TILE / POSITION_SCALE) +
                                 static_cast<int>(CHARACTER_HALF_HEIGHT * TAMANO_TILE));
}

void CalculadorPosicion::actualizar(const SDL2pp::Point& pies, const std::string& area,
                                    uint32_t tick, std::chrono::duration<float> duracion_tick,
                                    std::chrono::steady_clock::time_point ahora) {
    if (!this->tick || *this->tick != tick) {
        bool salta = !this->tick || area != area_id;
        desde = salta ? pies : actual;
        hasta = pies;
        llegada = ahora;
        this->tick = tick;
        area_id = area;
    }
    float avance = std::min(1.0f, (ahora - llegada) / duracion_tick);
    actual = SDL2pp::Point(desde.x + static_cast<int>(std::lround((hasta.x - desde.x) * avance)),
                           desde.y + static_cast<int>(std::lround((hasta.y - desde.y) * avance)));
}

const SDL2pp::Point& CalculadorPosicion::pies() const { return actual; }

const std::string& CalculadorPosicion::area() const { return area_id; }
