#ifndef MARKET_H
#define MARKET_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "server/game/commands/player_commerce.h"
#include "server/game/commerce/merchant.h"
#include "server/game/world/farm/farm.h"
#include "server/game/world/location.h"

// Los comerciantes del mercado y con quién habla cada jugador. Los animales que se compran
// aparecen en `animal_spawn` de la granja.
class Market: public PlayerCommerce {
public:
    Market(const Location& location, std::vector<Merchant> merchants, Farm& farm,
           const Position& animal_spawn);

    std::optional<ChatMessage> talk(Player& player, const Position& cell) override;
    TradeReply trade(Player& player, const std::string& command) override;

    // nullptr si no hay ninguno con ese nombre.
    const Merchant* find_merchant(const std::string& name) const;

private:
    const Location& location;
    std::vector<Merchant> merchants;
    Farm& farm;
    Position animal_spawn;
    // Con qué comerciante habla cada jugador, por id.
    std::map<uint32_t, const Merchant*> talking;

    // Si está en el mercado y le llega.
    bool can_reach(const Player& player, const Merchant& merchant) const;
    // Con quién habla; nullptr si con nadie. Si se alejó, la charla se termina.
    const Merchant* partner_of(const Player& player);
    TradeReply buy(Player& player, const Merchant& merchant, const std::string& name,
                   int quantity);
    TradeReply sell(Player& player, const Merchant& merchant, const std::string& name,
                    int quantity);
};

#endif  // MARKET_H
