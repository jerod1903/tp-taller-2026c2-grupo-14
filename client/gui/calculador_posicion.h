#ifndef CALCULADOR_POSICION_H
#define CALCULADOR_POSICION_H

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include <SDL2pp/SDL2pp.hh>

// Donde dibujar algo que mueve el servidor (un jugador, un animal). El servidor manda la posicion
// 30 veces por segundo: entre una y otra se dibuja en el camino, asi se mueve suave a cualquier fps
class CalculadorPosicion {
public:
    // El punto que manda el servidor, en milesimas de casilla, pasado a pixeles del mundo: el
    // centro de la caja de colision en x y su borde de abajo (los pies) en y
    static SDL2pp::Point pies_desde_servidor(uint32_t x, uint32_t y);
    // Se llama en cada frame. Si el tick es nuevo, `pies` pasa a ser el destino. Al aparecer o al
    // cambiar de area (un portal) salta, no camina hasta ahi
    void actualizar(const SDL2pp::Point& pies, const std::string& area, uint32_t tick,
                    std::chrono::steady_clock::time_point ahora);
    // En pixeles del mundo
    const SDL2pp::Point& pies() const;
    const std::string& area() const;

private:
    SDL2pp::Point desde;
    SDL2pp::Point hasta;
    SDL2pp::Point actual;
    // El tick del ultimo estado y cuando llego; vacio hasta el primero
    std::optional<uint32_t> tick;
    std::chrono::steady_clock::time_point llegada;
    std::string area_id;
};

#endif
