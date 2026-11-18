#ifndef PLAYER_COMMERCE_H
#define PLAYER_COMMERCE_H

#include <optional>
#include <string>

#include "common/game_dto.h"
#include "server/game/characters/player.h"
#include "server/game/world/position.h"

// Lo que contesta un comerciante a un comando, y si le cambió el inventario al jugador (para
// mandárselo).
struct TradeReply {
    ChatMessage message;
    bool inventory_changed;
};

// Hablar con los comerciantes y comerciar: lo que usan ClickCommand y ChatCommand.
class PlayerCommerce {
public:
    // Si en `cell` hay un comerciante del área del jugador, a su alcance, pasa a hablar con él: lo
    // cansa y devuelve el saludo. Si no, nullopt.
    virtual std::optional<ChatMessage> talk(Player& player, const Position& cell) = 0;
    // Un comando "/..." para el comerciante con el que habla. Si no habla con nadie, un aviso.
    virtual TradeReply trade(Player& player, const std::string& command) = 0;

    virtual ~PlayerCommerce() = default;
};

#endif  // PLAYER_COMMERCE_H
