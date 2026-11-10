#pragma once

#include <optional>

#include "common/map/MapDefinition.h"
#include "editor/Mine.h"
#include "editor/tools/PlaceTool.h"

class MapEditing;

// Coloca la entrada a la mina igual que PlaceTool coloca cualquier elemento (fantasma, click,
// terminar), pero la une con el primer piso y, si el mapa no tiene pisos, los crea del tamano
// elegido.
class EntranceTool: public PlaceTool {
public:
    explicit EntranceTool(MapEditing& editing);
    // El tamano de los pisos a crear; vacio si el mapa ya tiene pisos
    void setFloorSize(std::optional<AreaSize> size);

protected:
    bool place(stardew::maps::ElementType type, const stardew::maps::MapCell& cell) override;

private:
    std::optional<AreaSize> floorSize;
};
