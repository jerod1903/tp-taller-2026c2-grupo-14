#ifndef GAME_STORE_H
#define GAME_STORE_H

#include <cstdint>
#include <string>
#include <vector>

#include "common/lobby_dto.h"

struct GameMember {
    std::string username;
    std::string characterName;
    CharacterAppearance appearance;
};

// Lo que el lobby necesita saber de una partida, esté o no cargada en memoria.
struct GameRecord {
    uint32_t gameId;
    std::string gameName;
    std::string worldId;
    bool isMultiplayer;
    uint8_t maxPlayers;
    std::vector<GameMember> members;
    std::string inviteCode;  // vacío en las partidas single: no se puede entrar por código
};

// Dónde se guardan las partidas. El registry solo conoce esta interfaz: hoy la implementa
// InMemoryGameStore y después la persistencia en archivos, sin tocar al registry ni a la sesión.
class GameStore {
public:
    virtual std::vector<GameRecord> load_all() = 0;
    // Reemplaza la partida con el mismo gameId, o la agrega si no existe.
    virtual void save(const GameRecord& game) = 0;

    virtual ~GameStore() = default;
};

#endif  // GAME_STORE_H
