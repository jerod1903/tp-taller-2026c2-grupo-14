#ifndef CHAT_COMMAND_H
#define CHAT_COMMAND_H

#include <cstdint>
#include <string>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"

class Player;

// Un mensaje de chat de un jugador. El servidor decide a quien le llega:
//   "@nombre texto"  privado: solo al personaje con ese nombre, y una copia al que lo mando.
//                    Si no esta conectado, el que lo mando recibe un aviso del sistema.
//   "$nombre [payload]"  cheat: no le llega a nadie; si no existe, el que lo mando recibe un aviso.
//   cualquier otro   publico: a todos los jugadores de la partida.
// El texto se corta en MAX_CHAT_LENGTH y se le sacan los espacios de los costados; si queda
// vacio, no se manda nada.
class ChatCommand: public GameCommand {
public:
    ChatCommand(uint32_t player_id, const ChatRequest& request);
    void execute(GameContext& context) override;
    bool runs_while_day_ended() const override { return true; }

private:
    std::string text;

    // Manda un mensaje privado "@nombre texto" (sin la arroba ya separada)
    void send_private(GameContext& context, const std::string& author,
                      const std::string& message) const;
    // Activa el cheat "$nombre [payload]" (sin el $ ya separado)
    void run_cheat(GameContext& context, Player& author, const std::string& message) const;
};

#endif  // CHAT_COMMAND_H
