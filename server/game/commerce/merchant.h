#ifndef MERCHANT_H
#define MERCHANT_H

#include <string>
#include <variant>
#include <vector>

#include "common/game_dto.h"
#include "server/game/characters/animals/animal.h"
#include "server/game/world/position.h"

// Algo que se compra o se vende: un objeto del inventario o un animal de la granja. `name` es como
// se escribe en los comandos ("leche", "vaca").
struct Offer {
    std::string name;
    std::variant<ItemType, AnimalKind> goods;
    int price;
};

// Un NPC del mercado: está parado en una celda y tiene lo que vende y lo que compra.
class Merchant {
public:
    Merchant(std::string name, const Position& pos, std::vector<Offer> sells,
             std::vector<Offer> buys);

    const std::string& get_name() const;
    const Position& get_position() const;
    // Su celda o la de arriba: el sprite mide dos casillas y se le puede clickear la cabeza.
    bool occupies(const Position& cell) const;
    // Lo que vende o compra con ese nombre, o nullptr. Acepta el plural ("vacas").
    const Offer* find_sold(const std::string& name) const;
    const Offer* find_bought(const std::string& name) const;

    // El saludo, con los comandos que reconoce.
    std::string greeting() const;
    // "Vendo: vaca (80), gallina (20). Compro: vaca (40), gallina (10)."
    std::string catalog() const;

private:
    std::string name;
    Position pos;
    std::vector<Offer> sells;
    std::vector<Offer> buys;
};

#endif  // MERCHANT_H
