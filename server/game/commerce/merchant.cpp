#include "server/game/commerce/merchant.h"

#include <string>
#include <utility>
#include <vector>

namespace {

constexpr const char* COMMANDS =
        "Comandos: /consultar_catalogo, /comprar <producto> <cantidad>, "
        "/vender <producto> <cantidad>";
constexpr const char* NOTHING = "nada por ahora";
constexpr char PLURAL = 's';

const Offer* find_exact(const std::vector<Offer>& offers, const std::string& name) {
    for (const Offer& offer: offers) {
        if (offer.name == name) {
            return &offer;
        }
    }
    return nullptr;
}

// Tal cual y, si no está, sin la "s" del final: "vacas" -> "vaca".
const Offer* find(const std::vector<Offer>& offers, const std::string& name) {
    const Offer* found = find_exact(offers, name);
    if (found == nullptr && !name.empty() && name.back() == PLURAL) {
        found = find_exact(offers, name.substr(0, name.size() - 1));
    }
    return found;
}

// "vaca (80), gallina (20)", o NOTHING si no hay ninguna.
std::string list(const std::vector<Offer>& offers) {
    if (offers.empty()) {
        return NOTHING;
    }
    std::string text;
    for (const Offer& offer: offers) {
        text += (text.empty() ? "" : ", ") + offer.name + " (" + std::to_string(offer.price) + ")";
    }
    return text;
}

}  // namespace

Merchant::Merchant(std::string name, const Position& pos, std::vector<Offer> sells,
                   std::vector<Offer> buys):
        name(std::move(name)), pos(pos), sells(std::move(sells)), buys(std::move(buys)) {}

const std::string& Merchant::get_name() const { return name; }

const Position& Merchant::get_position() const { return pos; }

bool Merchant::occupies(const Position& cell) const {
    return cell.x == pos.x && (cell.y == pos.y || cell.y == pos.y - 1);
}

const Offer* Merchant::find_sold(const std::string& name) const { return find(sells, name); }

const Offer* Merchant::find_bought(const std::string& name) const { return find(buys, name); }

std::string Merchant::greeting() const { return "¡Hola! Soy " + name + ". " + COMMANDS; }

std::string Merchant::catalog() const {
    return "Vendo: " + list(sells) + ". Compro: " + list(buys) + ".";
}
