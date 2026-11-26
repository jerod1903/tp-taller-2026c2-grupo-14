#ifndef MINIMAPA_H
#define MINIMAPA_H

#include <cstdint>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/mapa-general/grilla_mapa.h"
#include "common/game_dto.h"

// Arriba a la izquierda, en el marco de madera del HUD: el área donde está el jugador, con un
// punto por cada jugador que esté en ella (el propio, más grande). En un interior no se dibuja.
class Minimapa {
public:
    Minimapa(SDL2pp::Renderer& renderer, GrillaMapa grilla);

    // escala_hud: la del HUD (escala_del_hud)
    void dibujar(SDL2pp::Renderer& renderer, const std::vector<PlayerState>& jugadores,
                 const PlayerState& propio, int escala_hud);

private:
    GrillaMapa grilla;
    SDL2pp::Texture textura;
    SDL2pp::Texture marco;
};

#endif
