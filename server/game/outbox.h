#ifndef OUTBOX_H
#define OUTBOX_H

#include <variant>

#include "common/game_dto.h"
#include "common/lib/queue.h"

// Lo que el Game le manda a un jugador; el Sender lo serializa.
using GameMessage = std::variant<LocationData, Snapshot, PlayerInfo, InventoryState, ChatMessage,
                                 FarmState, DungeonState, DayEnded, ChestState>;

using Outbox = Queue<GameMessage>;

#endif  // OUTBOX_H
