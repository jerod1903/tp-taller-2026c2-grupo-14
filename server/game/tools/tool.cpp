#include "server/game/tools/tool.h"

#include <map>

#include "server/game/tools/seed.h"
#include "server/game/tools/shovel.h"

const Tool* tool_for(ItemType item) {
    static const Shovel shovel;
    static const Seed strawberry(CropType::Strawberry);
    static const Seed scallion(CropType::Scallion);
    static const Seed potato(CropType::Potato);
    static const Seed pumpkin(CropType::Pumpkin);
    static const std::map<ItemType, const Tool*> tools = {
            {ItemType::Shovel, &shovel},
            {ItemType::StrawberrySeeds, &strawberry},
            {ItemType::ScallionSeeds, &scallion},
            {ItemType::PotatoSeeds, &potato},
            {ItemType::PumpkinSeeds, &pumpkin},
    };
    auto it = tools.find(item);
    return it != tools.end() ? it->second : nullptr;
}
