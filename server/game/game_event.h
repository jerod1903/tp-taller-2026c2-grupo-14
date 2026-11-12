#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <memory>

#include "common/lib/queue.h"
#include "server/game/commands/game_command.h"

// Todo lo que entra a la partida es un comando: los del cliente (Receiver) y los internos (lobby).
// shared_ptr porque la Queue solo copia: cada comando se libera cuando se suelta el último puntero,
// después de ejecutarlo o al destruirse la cola con comandos pendientes.
using Inbox = Queue<std::shared_ptr<GameCommand>>;

#endif  // GAME_EVENT_H
