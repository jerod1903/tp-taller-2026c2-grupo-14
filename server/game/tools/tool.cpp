#include "server/game/tools/tool.h"
#include "server/game/tools/pickaxe.h"

#include <map>

#include "server/game/tools/seed.h"
#include "server/game/tools/shovel.h"
#include "server/game/tools/watering_can.h"

const Tool* tool_for(ItemType item) {
    static const Shovel shovel;
    static const Pickaxe pickaxe;
    static const WateringCan watering_can;
    static const Seed strawberry(CropType::Strawberry);
    static const Seed scallion(CropType::Scallion);
    static const Seed potato(CropType::Potato);
    static const Seed pumpkin(CropType::Pumpkin);
    static const std::map<ItemType, const Tool*> tools = {
            {ItemType::Shovel, &shovel},
            {ItemType::Pickaxe, &pickaxe},
            {ItemType::WateringCan, &watering_can},
            {ItemType::StrawberrySeeds, &strawberry},
            {ItemType::ScallionSeeds, &scallion},
            {ItemType::PotatoSeeds, &potato},
            {ItemType::PumpkinSeeds, &pumpkin},
    };
    auto it = tools.find(item);
    return it != tools.end() ? it->second : nullptr;
}
