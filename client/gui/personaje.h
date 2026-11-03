#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "common/game_dto.h"

// 1 jugador en pantalla (solo dibuja, la pos la decide el sv)
class Personaje {
public:
    // sprite: compartido entre todos los personajes, se carga una sola vez
    Personaje(SDL2pp::Texture& sprite, int escala);
    void actualizar(const PlayerState& estado);
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) const;
    // En pixeles de pantalla (ya escalado), para la camara
    SDL2pp::Point centro() const;
    // Id del area del mapa en la que esta
    const std::string& area() const;

private:
    SDL2pp::Texture& sprite;
    // Esquina superior izquierda del sprite, en pixeles del tileset (sin escalar)
    SDL2pp::Point posicion;
    std::string area_id;
    int escala;
};

#endif
