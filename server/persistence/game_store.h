#ifndef GAME_STORE_H
#define GAME_STORE_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "common/lobby_dto.h"
#include "server/persistence/saved_game.h"

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
    std::vector<GameMember> members;  // el primero es el creador
    // Vacío en las partidas single: no se puede entrar por código.
    std::string inviteCode;
};

// Dónde se guardan las partidas: el índice (un GameRecord por partida, lo que el servidor carga al
// arrancar) y el estado de cada una (lo que se carga recién cuando alguien la abre). El registry
// solo conoce esta interfaz: la implementan FileGameStore (en archivos) e InMemoryGameStore (para
// los tests).
class GameStore {
public:
    virtual std::vector<GameRecord> load_all() = 0;
    // Reemplaza la partida con el mismo gameId, o la agrega si no existe.
    virtual void save(const GameRecord& game) = 0;

    // El estado guardado de la partida, o std::nullopt si todavía no se guardó ninguno (no pasó
    // ningún día).
    virtual std::optional<SavedGame> load_game(uint32_t game_id) = 0;
    // Reemplaza el estado guardado de la partida.
    virtual void save_game(uint32_t game_id, const SavedGame& game) = 0;

    virtual ~GameStore() = default;
};

#endif  // GAME_STORE_H
