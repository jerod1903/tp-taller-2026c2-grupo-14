#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include <cstdint>
#include <memory>

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
    
    virtual ~GameContext() = default;
};

#endif  // GAME_CONTEXT_H
