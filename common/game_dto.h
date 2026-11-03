#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <cstdint>
#include <string>
#include <vector>

// Mensajes del servidor durante la partida: cada uno va precedido por su tag. Lo que manda el
// cliente es un Request (common/requests.h).
enum class GameMsg : uint8_t {
    LocationData,
    Snapshot,
};

// El área en la que está el jugador. Se manda al entrar y cada vez que cambia.
struct LocationData {
    std::string worldId;
};

// Estado de la partida en un tick. Por ahora solo quiénes están conectados.
struct Snapshot {
    uint32_t tick;
    std::vector<uint32_t> players;  // playerIds
};

#endif  // GAME_DTO_H
