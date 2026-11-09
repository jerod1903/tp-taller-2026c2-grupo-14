#include "editor/Elements.h"

#include <string>
#include <vector>

namespace maps = stardew::maps;

bool removeElement(maps::MapDefinition& map, const std::string& id) {
    if (std::erase_if(map.elements, [&id](const auto& e) { return e.id == id; }) == 0) {
        return false;
    }
    std::erase_if(map.transitions, [&id](const auto& t) { return t.triggerElementId == id; });
    return true;
}
