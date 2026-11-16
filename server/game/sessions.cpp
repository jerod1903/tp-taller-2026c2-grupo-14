#include "server/game/sessions.h"

#include <stdexcept>
#include <utility>

#include "common/logger.h"
#include "server/config/server_config.h"

Sessions::Sessions(std::string world_id, World& world):
        world_id(std::move(world_id)), world(world) {}

void Sessions::add_player(uint32_t player_id, const GameMember& member,
                          std::shared_ptr<Outbox> outbox) {
    remove(player_id);
    world.spawn_player(player_id, member.characterName);
    send(*outbox, LocationData{world_id, static_cast<uint16_t>(server_config().game.tick_rate),
                               server_config().game.max_players});
    PlayerInfo info{player_id, member.characterName, member.appearance};
    for (const auto& [other_id, other]: players) {
        send(*outbox, other.info);
        send(*other.outbox, info);
    }
    send(*outbox, info);
    players.emplace(player_id, ConnectedPlayer{std::move(outbox), std::move(info)});
}

void Sessions::remove_player(uint32_t player_id) {
    auto player = players.find(player_id);
    if (player == players.end()) {
        return;
    }
    try {
        player->second.outbox->close();
    } catch (const std::runtime_error&) {}
    remove(player_id);
}

void Sessions::broadcast(const GameMessage& message) {
    for (const auto& [player_id, player]: players) {
        send(*player.outbox, message);
    }
}

void Sessions::remove(uint32_t player_id) {
    auto player = players.find(player_id);
    if (player != players.end()) {
        world.remove_player(player_id);
        players.erase(player);
    }
}

void Sessions::send(Outbox& outbox, const GameMessage& message) {
    try {
        if (!outbox.try_push(message)) {
            Logger::debug("outbox llena: se descarta un mensaje");
        }
    } catch (const ClosedQueue&) {}
}
