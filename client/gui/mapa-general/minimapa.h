#ifndef MINIMAPA_H
#define MINIMAPA_H

#include <cstdint>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/game/estado_juego.h"
#include "client/gui/sprites/constructor_capas.h"
#include "client/mapa-general/grilla_mapa.h"

// Arriba a la izquierda, en el marco de madera del HUD: el área donde está el jugador, con la cara
// de cada jugador que esté en ella (la propia, más grande). En un interior no se dibuja.
class Minimapa {
public:
    Minimapa(SDL2pp::Renderer& renderer, GrillaMapa grilla);

    // caras: de donde salen las caras de los jugadores. escala_hud: la del HUD (escala_del_hud)
    void dibujar(SDL2pp::Renderer& renderer, const EstadoJuego& estado, ConstructorCapas& caras,
                 int escala_hud);

private:
    GrillaMapa grilla;
    SDL2pp::Texture textura;
    SDL2pp::Texture marco;
};

#endif
