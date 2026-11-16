#ifndef SESSIONS_H
#define SESSIONS_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "common/game_dto.h"
#include "server/game/commands/player_sessions.h"
#include "server/game/outbox.h"
#include "server/game/world/world.h"

// Los jugadores conectados a una partida: su outbox y su info. Al entrar crea su personaje en el
// World y le manda la ubicación, quiénes están y su inventario; al salir lo saca.
class Sessions: public PlayerSessions {
public:
    Sessions(std::string world_id, World& world);

    void add_player(uint32_t player_id, const GameMember& member,
                    std::shared_ptr<Outbox> outbox) override;
    void remove_player(uint32_t player_id) override;

    void send_to(uint32_t player_id, const GameMessage& message) override;
    void broadcast(const GameMessage& message) override;
    std::optional<uint32_t> find_player_by_name(const std::string& character_name) override;

private:
    struct ConnectedPlayer {
        std::shared_ptr<Outbox> outbox;
        PlayerInfo info;
    };
    const std::string world_id;
    World& world;
    std::map<uint32_t, ConnectedPlayer> players;

    // Saca su personaje del mundo sin cerrar la outbox.
    void remove(uint32_t player_id);
    // Con la outbox llena o cerrada el mensaje se descarta: la partida nunca se bloquea.
    static void send(Outbox& outbox, const GameMessage& message);
};

#endif  // SESSIONS_H
