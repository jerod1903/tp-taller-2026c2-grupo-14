#ifndef GIFTS_H
#define GIFTS_H

#include <cstdint>
#include <map>
#include <optional>

#include "common/game_dto.h"
#include "server/game/commands/player_characters.h"
#include "server/game/commands/player_gifts.h"

// Los regalos ofrecidos que todavía no se contestaron: uno por receptor (uno nuevo reemplaza al
// anterior). La pila sigue en el inventario del que regala hasta que se acepta.
class Gifts: public PlayerGifts {
public:
    explicit Gifts(PlayerCharacters& characters);

    std::optional<GiftReply> offer(Player& giver, const Position& cell) override;
    GiftReply answer(Player& receiver, bool accept) override;

private:
    struct Pending {
        uint32_t giver_id;
        uint8_t slot;
        // Lo que había en el slot al ofrecerlo: si cambió, el regalo ya no vale
        InventorySlot gift;
    };

    PlayerCharacters& characters;
    std::map<uint32_t, Pending> pending;
};

#endif  // GIFTS_H
