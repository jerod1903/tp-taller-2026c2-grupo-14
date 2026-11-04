#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "common/game_dto.h"

#include "sprite.h"

// 1 jugador en pantalla (solo dibuja, la pos la decide el sv). El servidor manda la posicion 30
// veces por segundo: entre una y otra se dibuja en el camino, asi se mueve suave a cualquier fps
class Personaje {
public:
    // textura: compartida entre todos los personajes, se carga una sola vez
    explicit Personaje(SDL2pp::Texture& textura);
    // Se llama en cada frame. Si el estado es de un tick nuevo, su posicion pasa a ser el destino
    void actualizar(const PlayerState& estado, uint32_t tick,
                    std::chrono::steady_clock::time_point ahora);
    // Donde dibujarlo en este frame, en pixeles del mundo
    Sprite sprite() const;
    // En pixeles del mundo, para la camara
    SDL2pp::Point centro() const;
    // Id del area del mapa en la que esta
    const std::string& area() const;

private:
    SDL2pp::Texture& textura;
    // Los pies, en pixeles del mundo: desde donde va, hacia donde y donde se dibuja ahora
    SDL2pp::Point desde;
    SDL2pp::Point hasta;
    SDL2pp::Point pies;
    // El tick del ultimo estado y cuando llego; vacio hasta el primero
    std::optional<uint32_t> tick;
    std::chrono::steady_clock::time_point llegada;
    std::string area_id;
};

#endif
