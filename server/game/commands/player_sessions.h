#ifndef PLAYER_SESSIONS_H
#define PLAYER_SESSIONS_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "server/game/outbox.h"
#include "server/persistence/game_store.h"

// Los jugadores conectados: entrar, salir y mandarles mensajes. Lo usan JoinCommand, LeaveCommand
// y los que le responden al jugador (Chat, Interact, SelectSlot).
class PlayerSessions {
public:
    // Crea su personaje en su cama de la cabaña; si ya estaba, lo reemplaza.
    virtual void add_player(uint32_t player_id, const GameMember& member,
                            std::shared_ptr<Outbox> outbox) = 0;
    // Cierra su outbox: el Sender termina de vaciarla y ya no le llega nada más.
    virtual void remove_player(uint32_t player_id) = 0;

    // A la outbox de ese jugador; si no está conectado, no hace nada.
    virtual void send_to(uint32_t player_id, const GameMessage& message) = 0;
    // A todos los jugadores conectados.
    virtual void broadcast(const GameMessage& message) = 0;
    // El id del jugador conectado cuyo personaje se llama así; nullopt si no hay ninguno.
    virtual std::optional<uint32_t> find_player_by_name(const std::string& character_name) = 0;

    virtual ~PlayerSessions() = default;
};

#endif  // PLAYER_SESSIONS_H
