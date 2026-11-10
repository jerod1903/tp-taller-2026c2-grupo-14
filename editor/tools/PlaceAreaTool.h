#pragma once

#include <functional>
#include <optional>

#include "common/map/MapDefinition.h"
#include "editor/MapHelper.h"
#include "editor/tools/Ghost.h"
#include "editor/tools/Tool.h"

class MapEditing;

// Pone una granja o un pueblo nuevo, del tamano elegido, con su esquina de arriba a la izquierda
// donde se hace click. Mientras se mueve el mouse avisa el fantasma del area, rojo si pisa otra;
// al ponerla o al cancelar avisa que termino. Un click donde no entra no hace nada.
class PlaceAreaTool: public Tool {
public:
    explicit PlaceAreaTool(MapEditing& editing);
    void setArea(stardew::maps::AreaType type, const AreaSize& size);
    void onPress(const stardew::maps::MapCell& cell) override;
    void onHover(const stardew::maps::MapCell& cell) override;
    void onCancel() override;
    // Cambio el fantasma: vacio si no hay que mostrarlo
    void onGhostChanged(std::function<void(const std::optional<AreaGhost>&)> callback);
    // Se puso el area o se cancelo
    void onDone(std::function<void()> callback);

private:
    struct NewArea {
        stardew::maps::AreaType type;
        AreaSize size;
    };

    void showGhost(const std::optional<AreaGhost>& ghost);
    void finish();

    MapEditing& editing;
    std::optional<NewArea> area;
    std::function<void(const std::optional<AreaGhost>&)> ghostChanged;
    std::function<void()> done;
};
