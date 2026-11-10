#pragma once

#include "common/map/MapDefinition.h"
#include "editor/tools/Tool.h"

class MapEditing;

// Pinta con el tile elegido la casilla del click y las que se recorren arrastrando.
class PaintTool: public Tool {
public:
    explicit PaintTool(MapEditing& editing);
    void setTile(stardew::maps::TileType tile);
    void onPress(const stardew::maps::MapCell& cell) override;
    void onMove(const stardew::maps::MapCell& cell) override;

private:
    MapEditing& editing;
    stardew::maps::TileType tile = stardew::maps::TileType::Grass;
};
