#include "server/game/storage/item_names.h"

#include <map>

std::string item_name(ItemType item) {
    static const std::map<ItemType, std::string> names = {
            {ItemType::Pickaxe, "pico"},
            {ItemType::WateringCan, "regadera"},
            {ItemType::Shovel, "pala"},
            {ItemType::Sword, "espada"},
            {ItemType::Milk, "leche"},
            {ItemType::StrawberrySeeds, "semillas de frutilla"},
            {ItemType::ScallionSeeds, "semillas de cebolla de verdeo"},
            {ItemType::PotatoSeeds, "semillas de papa"},
            {ItemType::PumpkinSeeds, "semillas de calabaza"},
            {ItemType::Strawberry, "frutilla"},
            {ItemType::Scallion, "cebolla de verdeo"},
            {ItemType::Potato, "papa"},
            {ItemType::Pumpkin, "calabaza"},
            {ItemType::Egg, "huevo"},
            {ItemType::Coal, "carbón"},
            {ItemType::Lapis, "lapislázuli"},
            {ItemType::Silver, "plata"},
            {ItemType::GoldOre, "oro"},
            {ItemType::Chest, "cofre"},
    };
    auto it = names.find(item);
    return it != names.end() ? it->second : "objeto";
}
