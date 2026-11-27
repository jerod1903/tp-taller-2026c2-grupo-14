#include "client/gui/mapa/transicion_area.h"

#include <limits>

TransicionArea::TransicionArea(std::chrono::duration<float> duracion): duracion(duracion.count()) {}

void TransicionArea::empezar(std::chrono::steady_clock::time_point ahora) { inicio = ahora; }

bool TransicionArea::activa(std::chrono::steady_clock::time_point ahora) const {
    return inicio && transcurrido(ahora) < duracion;
}

bool TransicionArea::oscureciendo(std::chrono::steady_clock::time_point ahora) const {
    return activa(ahora) && transcurrido(ahora) < duracion / 2;
}

uint8_t TransicionArea::opacidad(std::chrono::steady_clock::time_point ahora) const {
    if (!activa(ahora)) {
        return 0;
    }
    float mitad = duracion / 2;
    float t = transcurrido(ahora);
    float avance;
    if (t < mitad) {
        avance = t / mitad;
    } else {
        avance = (duracion - t) / mitad;
    }
    return static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() * avance);
}

float TransicionArea::transcurrido(std::chrono::steady_clock::time_point ahora) const {
    return std::chrono::duration<float>(ahora - *inicio).count();
}
