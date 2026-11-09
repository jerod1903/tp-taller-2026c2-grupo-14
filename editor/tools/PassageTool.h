#pragma once

#include "common/map/MapDefinition.h"
#include "editor/tools/Tool.h"

class MapEditing;

// Un click en el borde entre dos areas pone un pasaje, o lo quita si ya hay uno.
class PassageTool: public Tool {
public:
    explicit PassageTool(MapEditing& editing);
    void onPress(const stardew::maps::MapCell& cell) override;

private:
    MapEditing& editing;
};
