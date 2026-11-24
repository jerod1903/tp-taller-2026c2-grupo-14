#include "server/game/commerce/market.h"

#include <charconv>
#include <cmath>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include "server/config/server_config.h"
#include "server/consts.h"

namespace {

constexpr int MAX_QUANTITY = 99;
// Para medir hasta el centro de la celda del comerciante.
constexpr float CELL_CENTER = 0.5f;
constexpr const char* CATALOG_COMMAND = "/consultar_catalogo";
constexpr const char* BUY_COMMAND = "/comprar";
constexpr const char* SELL_COMMAND = "/vender";
constexpr const char* NOT_TALKING = "Hacé click en un comerciante para hablarle";
constexpr const char* BAD_QUANTITY = "Decime qué producto y una cantidad de 1 a 99";

TradeReply reply(const Merchant& merchant, const std::string& text, bool changed = false) {
    return TradeReply{ChatMessage{ChatKind::Merchant, merchant.get_name(), text}, changed};
}

// La cantidad escrita: 1 si no hay; nullopt si no es un número de 1 a MAX_QUANTITY.
std::optional<int> parse_quantity(const std::string& text) {
    if (text.empty()) {
        return 1;
    }
    int quantity = 0;
    const char* end = text.data() + text.size();
    auto [last, error] = std::from_chars(text.data(), end, quantity);
    if (error != std::errc() || last != end || quantity < 1 || quantity > MAX_QUANTITY) {
        return std::nullopt;
    }
    return quantity;
}

int max_animals(AnimalKind kind) {
    return kind == AnimalKind::Cow ? server_config().animals.cow.max_per_farm :
                                     server_config().animals.chicken.max_per_farm;
}

}  // namespace

Market::Market(const Location& location, std::vector<Merchant> merchants, Farm& farm,
               const Position& animal_spawn):
        location(location),
        merchants(std::move(merchants)),
        farm(farm),
        animal_spawn(animal_spawn) {}

std::optional<ChatMessage> Market::talk(Player& player, const Position& cell) {
    for (const Merchant& merchant: merchants) {
        if (merchant.occupies(cell) && can_reach(player, merchant)) {
            talking[player.get_id()] = &merchant;
            player.spend_energy(server_config().energy.talk);
            return ChatMessage{ChatKind::Merchant, merchant.get_name(), merchant.greeting()};
        }
    }
    return std::nullopt;
}

TradeReply Market::trade(Player& player, const std::string& command) {
    const Merchant* merchant = partner_of(player);
    if (merchant == nullptr) {
        return TradeReply{ChatMessage{ChatKind::System, "", NOT_TALKING}, false};
    }
    std::istringstream words(command);
    std::string action;
    std::string product;
    std::string amount;
    words >> action >> product >> amount;
    if (action == CATALOG_COMMAND) {
        return reply(*merchant, merchant->catalog());
    }
    if (action != BUY_COMMAND && action != SELL_COMMAND) {
        return reply(*merchant, merchant->greeting());
    }
    std::optional<int> quantity = parse_quantity(amount);
    if (product.empty() || !quantity) {
        return reply(*merchant, BAD_QUANTITY);
    }
    if (action == BUY_COMMAND) {
        return buy(player, *merchant, product, *quantity);
    }
    return sell(player, *merchant, product, *quantity);
}

const Merchant* Market::find_merchant(const std::string& name) const {
    for (const Merchant& merchant: merchants) {
        if (merchant.get_name() == name) {
            return &merchant;
        }
    }
    return nullptr;
}

bool Market::can_reach(const Player& player, const Merchant& merchant) const {
    if (&player.get_location() != &location) {
        return false;
    }
    float dx = player.get_x() - (static_cast<float>(merchant.get_position().x) + CELL_CENTER);
    float dy = player.get_y() - (static_cast<float>(merchant.get_position().y) + CELL_CENTER);
    return std::hypot(dx, dy) <= server_config().market.reach;
}

const Merchant* Market::partner_of(const Player& player) {
    auto it = talking.find(player.get_id());
    if (it == talking.end()) {
        return nullptr;
    }
    if (!can_reach(player, *it->second)) {
        talking.erase(it);
        return nullptr;
    }
    return it->second;
}

TradeReply Market::buy(Player& player, const Merchant& merchant, const std::string& name,
                       int quantity) {
    const Offer* offer = merchant.find_sold(name);
    if (offer == nullptr) {
        return reply(merchant, "No vendo " + name + ". " + merchant.catalog());
    }
    int total = offer->price * quantity;
    if (player.get_gold() < total) {
        return reply(merchant, "Son " + std::to_string(total) + " monedas y no te alcanza");
    }
    if (const AnimalKind* kind = std::get_if<AnimalKind>(&offer->goods)) {
        if (farm.count_animals(*kind) + quantity > max_animals(*kind)) {
            return reply(merchant, "No entran más en tu granja");
        }
        player.add_gold(-total);
        for (int i = 0; i < quantity; ++i) {
            farm.add_animal(*kind, animal_spawn);
        }
        return reply(merchant, "¡Listo! Ya están en tu granja");
    }
    ItemType item = std::get<ItemType>(offer->goods);
    if (player.room_for(item) < quantity) {
        return reply(merchant, "No te entra en el inventario");
    }
    player.add_gold(-total);
    player.receive(item, static_cast<uint8_t>(quantity));
    return reply(merchant, "¡Listo! Pagaste " + std::to_string(total) + " monedas", true);
}

TradeReply Market::sell(Player& player, const Merchant& merchant, const std::string& name,
                        int quantity) {
    const Offer* offer = merchant.find_bought(name);
    if (offer == nullptr) {
        return reply(merchant, "No compro " + name + ". " + merchant.catalog());
    }
    int total = offer->price * quantity;
    bool inventory_changed = false;
    if (const AnimalKind* kind = std::get_if<AnimalKind>(&offer->goods)) {
        if (farm.count_animals(*kind) < quantity) {
            return reply(merchant, "No tenés tantos en tu granja");
        }
        farm.remove_animals(*kind, quantity);
    } else {
        ItemType item = std::get<ItemType>(offer->goods);
        if (player.count(item) < quantity) {
            return reply(merchant, "No tenés tantos");
        }
        // Cada calidad se paga a su multiplicador; se redondea por calidad, no por unidad
        total = 0;
        for (const auto& [quality, amount]: player.take(item, quantity)) {
            total += static_cast<int>(std::lround(
                    offer->price * amount *
                    ServerConstants::QUALITY_PRICE_MULTIPLIER.at(quality)));
        }
        inventory_changed = true;
    }
    player.add_gold(total);
    return reply(merchant, "¡Gracias! Te pago " + std::to_string(total) + " monedas",
                 inventory_changed);
}
