#ifndef ESTADO_JUEGO_H
#define ESTADO_JUEGO_H

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "common/game_dto.h"

struct MensajeChat {
    ChatMessage mensaje;
    std::chrono::steady_clock::time_point llegada;
};

struct EstadoJuego {
    uint32_t id_propio;  // el playerId de este jugador
    // Nombre y apariencia de cada jugador, por playerId. Llegan al entrar y cuando entra otro.
    std::map<uint32_t, PlayerInfo> info_jugadores;
    std::optional<std::string> mundo;
    std::vector<PlayerState> jugadores;
    uint32_t tick = 0;
    std::vector<EntityState> entidades = {};
    // Cuanto dura un tick del servidor y cuantos jugadores entran: llegan con LocationData
    std::chrono::duration<float> duracion_tick{};
    int max_jugadores = 0;
    InventoryState inventario{};
    std::vector<MensajeChat> chat = {};
    FarmState granja{};
};

#endif
