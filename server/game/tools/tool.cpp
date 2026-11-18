#include "server/game/tools/tool.h"

#include <map>

#include "server/game/tools/shovel.h"

const Tool* tool_for(ItemType item) {
    static const Shovel shovel;
    static const std::map<ItemType, const Tool*> tools = {
            {ItemType::Shovel, &shovel},
    };
    auto it = tools.find(item);
    return it != tools.end() ? it->second : nullptr;
}
