#ifndef CALCULADOR_POSICION_H
#define CALCULADOR_POSICION_H

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include <SDL2pp/SDL2pp.hh>

class CalculadorPosicion {
public:
    static SDL2pp::Point pies_desde_servidor(uint32_t x, uint32_t y);
    void actualizar(const SDL2pp::Point& pies, const std::string& area, uint32_t tick,
                    std::chrono::duration<float> duracion_tick,
                    std::chrono::steady_clock::time_point ahora);
    const SDL2pp::Point& pies() const;
    const std::string& area() const;

private:
    SDL2pp::Point desde;
    SDL2pp::Point hasta;
    SDL2pp::Point actual;
    std::optional<uint32_t> tick;
    std::chrono::steady_clock::time_point llegada;
    std::string area_id;
};

#endif
