#include "editor/Areas.h"

#include <algorithm>
#include <string>
#include <vector>

#include "editor/Passages.h"

namespace maps = stardew::maps;

void moveAreas(maps::MapDefinition& map, const std::vector<std::string>& areaIds, int dx, int dy) {
    // Los pasajes se buscan con las areas donde estan: hay que quitarlos antes de correrlas
    for (const std::string& doorId: passagesBrokenBy(map, areaIds, dx, dy)) {
        removePassage(map, doorId);
    }
    for (maps::MapArea& area: map.areas) {
        if (std::find(areaIds.begin(), areaIds.end(), area.id) != areaIds.end()) {
            area.offsetX += dx;
            area.offsetY += dy;
        }
    }
}
