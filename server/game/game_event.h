#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <cstdint>
#include <memory>
#include <variant>

#include "common/lib/queue.h"
#include "common/requests.h"
#include "server/game/outbox.h"
#include "server/persistence/game_store.h"

struct PlayerJoined {
    uint32_t playerId;
    GameMember member;
    std::shared_ptr<Outbox> outbox;
};

struct PlayerAction {
    uint32_t playerId;
    Request request;
};

struct PlayerLeft {
    uint32_t playerId;
};

using GameEvent = std::variant<PlayerJoined, PlayerAction, PlayerLeft>;
using Inbox = Queue<GameEvent>;

#endif  // GAME_EVENT_H
