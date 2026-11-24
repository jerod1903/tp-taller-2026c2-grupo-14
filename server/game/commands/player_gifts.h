#ifndef PLAYER_GIFTS_H
#define PLAYER_GIFTS_H

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "common/game_dto.h"
#include "server/game/world/position.h"

class Player;

// Lo que hay que avisar después de un regalo: mensajes por el chat y a quiénes les cambió el
// inventario (para mandárselo).
struct GiftReply {
    std::vector<std::pair<uint32_t, ChatMessage>> messages;
    std::vector<uint32_t> inventory_changed;
};

// Regalar la pila del slot elegido a otro jugador, que la acepta o la rechaza por el chat: lo que
// usan ClickCommand y ChatCommand.
class PlayerGifts {
public:
    // Si en `cell` hay otro jugador a su alcance, le ofrece la pila que tiene elegida. nullopt si
    // ahí no hay nadie a quien regalarle (el click sigue con la herramienta).
    virtual std::optional<GiftReply> offer(Player& giver, const Position& cell) = 0;
    // Contesta el regalo que le ofrecieron: si acepta, recibe lo que le entre de la pila.
    virtual GiftReply answer(Player& receiver, bool accept) = 0;

    virtual ~PlayerGifts() = default;
};

#endif  // PLAYER_GIFTS_H
