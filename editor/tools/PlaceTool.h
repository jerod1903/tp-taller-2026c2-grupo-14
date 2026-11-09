#pragma once

#include <functional>
#include <optional>

#include "common/map/MapDefinition.h"
#include "editor/tools/Tool.h"

class MapEditing;

// Lo que se muestra bajo el mouse antes de colocar: el elemento apoyado con la esquina de abajo a
// la izquierda de su base en la casilla, y si entra ahi
struct Ghost {
    stardew::maps::ElementType type;
    stardew::maps::MapCell bottomLeft;
    int width;
    int height;
    bool fits;
};

// Coloca un elemento del tipo elegido donde se hace click. Mientras se mueve el mouse avisa el
// fantasma; al colocar o al cancelar avisa que termino. Un click donde no entra no hace nada.
class PlaceTool: public Tool {
public:
    explicit PlaceTool(MapEditing& editing);
    void setType(stardew::maps::ElementType type);
    void onPress(const stardew::maps::MapCell& cell) override;
    void onHover(const stardew::maps::MapCell& cell) override;
    void onCancel() override;
    // Cambio el fantasma: vacio si no hay que mostrarlo (fuera de toda area o ya termino)
    void onGhostChanged(std::function<void(const std::optional<Ghost>&)> callback);
    // Se coloco el elemento o se cancelo
    void onDone(std::function<void()> callback);

private:
    void showGhost(const std::optional<Ghost>& ghost);
    void finish();

    MapEditing& editing;
    std::optional<stardew::maps::ElementType> type;
    std::function<void(const std::optional<Ghost>&)> ghostChanged;
    std::function<void()> done;
};
