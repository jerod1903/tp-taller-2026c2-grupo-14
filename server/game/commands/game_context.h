#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "server/game/characters/animals/animal.h"
#include "server/game/characters/player.h"
#include "server/game/outbox.h"
#include "server/persistence/game_store.h"

class GameContext {
public:
    // nullptr si el jugador no está en la partida.
    virtual Player* find_player(uint32_t player_id) = 0;
    // Crea su personaje en el spawn de la granja; si ya estaba, lo reemplaza.
    virtual void add_player(uint32_t player_id, const GameMember& member,
                            std::shared_ptr<Outbox> outbox) = 0;
    // Cierra su outbox: el Sender termina de vaciarla y ya no le llega nada más.
    virtual void remove_player(uint32_t player_id) = 0;

       
    // Enviar un mensaje directo a la outbox de un jugador específico.
    virtual void send_to(uint32_t player_id, const GameMessage& message) = 0;
    // Manda el mensaje a todos los jugadores conectados a la partida.
    virtual void broadcast(const GameMessage& message) = 0;
    // El id del jugador conectado cuyo personaje se llama así; nullopt si no hay ninguno.
    virtual std::optional<uint32_t> find_player_by_name(const std::string& character_name) = 0;
    // Los animales de la granja (para ordeñar).
    virtual const std::vector<std::unique_ptr<Animal>>& farm_animals() = 0;
    
    virtual ~GameContext() = default;
};

#endif  // GAME_CONTEXT_H
