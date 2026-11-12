#ifndef ESTADO_JUEGO_H
#define ESTADO_JUEGO_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "common/game_dto.h"

// Lo que el cliente sabe de la partida, armado con lo que manda el servidor. Son solo datos: como
// se dibujan lo decide la Interfaz.
struct EstadoJuego {
    uint32_t id_propio;                  // el playerId de este jugador
    // Nombre y apariencia de cada jugador, por playerId. Llegan al entrar y cuando entra otro.
    std::map<uint32_t, PlayerInfo> info_jugadores;
    std::optional<std::string> mundo;    // llega con LocationData; vacio hasta entonces
    std::vector<PlayerState> jugadores;  // los del ultimo Snapshot, en cualquier area
    uint32_t tick = 0;                   // el del ultimo Snapshot
    // Lo que se dibuja animado (jugadores y, cuando existan, animales), en cualquier area
    std::vector<EntityState> entidades = {};
    InventoryState inventario{};
};

#endif
