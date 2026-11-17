#ifndef TRANSICION_AREA_H
#define TRANSICION_AREA_H

#include <chrono>
#include <cstdint>
#include <optional>

class TransicionArea {
public:
    explicit TransicionArea(std::chrono::duration<float> duracion);
    void empezar(std::chrono::steady_clock::time_point ahora);
    bool activa(std::chrono::steady_clock::time_point ahora) const;
    bool oscureciendo(std::chrono::steady_clock::time_point ahora) const;
    uint8_t opacidad(std::chrono::steady_clock::time_point ahora) const;

private:
    float transcurrido(std::chrono::steady_clock::time_point ahora) const;
    float duracion;
    std::optional<std::chrono::steady_clock::time_point> inicio;
};

#endif
