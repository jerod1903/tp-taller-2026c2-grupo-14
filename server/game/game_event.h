#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <memory>

#include "common/lib/queue.h"
#include "server/game/commands/game_command.h"

// shared_ptr porque la Queue solo copia: el comando se libera al soltar el último puntero.
using Inbox = Queue<std::shared_ptr<GameCommand>>;

#endif  // GAME_EVENT_H
