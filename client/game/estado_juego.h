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
    uint16_t dia = 0;             // 0 hasta el primer snapshot
    uint16_t minuto_del_dia = 0;  // minutos de juego desde las 6:00
    std::vector<EntityState> entidades = {};
    // Cuanto dura un tick del servidor y cuantos jugadores entran: llegan con LocationData
    std::chrono::duration<float> duracion_tick{};
    int max_jugadores = 0;
    int energia_maxima = 0;  // llega con LocationData
    int vida_maxima = 0;     // llega con LocationData
    InventoryState inventario{};
    std::vector<MensajeChat> chat = {};
    FarmState granja{};
    std::map<std::string, DungeonState> mina{};
    // Mientras se vota pasar de día: cómo va la votación. Se va cuando llega un snapshot del día
    // siguiente.
    std::optional<DayEnded> fin_del_dia = std::nullopt;
    bool listo_para_manana = false;  // ya dio el Ok
    // El contenido del cofre que tiene abierto; nullopt si no tiene ninguno
    std::optional<std::vector<InventorySlot>> cofre = std::nullopt;
    // Llegó GameCompleted y todavía no cerró la felicitación (es solo del cliente)
    bool juego_completado = false;
};

#endif
