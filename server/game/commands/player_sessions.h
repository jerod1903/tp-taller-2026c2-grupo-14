#ifndef PLAYER_SESSIONS_H
#define PLAYER_SESSIONS_H

#include <cstdint>
#include <memory>

#include "server/game/outbox.h"
#include "server/persistence/game_store.h"

// Entrar y salir de la partida: lo que usan JoinCommand y LeaveCommand.
class PlayerSessions {
public:
    // Crea su personaje en su cama de la cabaña; si ya estaba, lo reemplaza.
    virtual void add_player(uint32_t player_id, const GameMember& member,
                            std::shared_ptr<Outbox> outbox) = 0;
    // Cierra su outbox: el Sender termina de vaciarla y ya no le llega nada más.
    virtual void remove_player(uint32_t player_id) = 0;

    virtual ~PlayerSessions() = default;
};

#endif  // PLAYER_SESSIONS_H
