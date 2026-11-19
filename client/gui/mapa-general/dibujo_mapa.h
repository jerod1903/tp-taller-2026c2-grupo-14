#ifndef DIBUJO_MAPA_H
#define DIBUJO_MAPA_H

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/mapa-general/grilla_mapa.h"
#include "common/game_dto.h"

// Lo que comparten el mapa general (M) y el minimapa del HUD.

// La grilla como textura: una casilla, un píxel, para estirarla al dibujar. Con la grilla vacía (un
// mapa sin áreas), un píxel negro.
SDL2pp::Texture textura_de_grilla(SDL2pp::Renderer& renderer, const GrillaMapa& grilla);

// El centro del jugador en casillas de la grilla (su área corrida a su lugar), o vacío si su área
// no está en el mapa general: está en un interior
std::optional<SDL_FPoint> jugador_en_grilla(const GrillaMapa& grilla, const PlayerState& jugador);

// Cada jugador con si es el propio; el propio al final, para que su punto quede encima
void recorrer_jugadores(const std::vector<PlayerState>& jugadores, uint32_t id_propio,
                        const std::function<void(const PlayerState&, bool)>& hacer);

// Un punto blanco con borde negro, de ese radio en píxeles
void dibujar_punto_jugador(SDL2pp::Renderer& renderer, const SDL_FPoint& centro, int radio);

#endif
