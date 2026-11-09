#pragma once

#include <functional>
#include <optional>

#include "common/map/MapDefinition.h"
#include "editor/tools/Ghost.h"
#include "editor/tools/Tool.h"

class MapEditing;

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

protected:
    // Lo que hace el click; devuelve si se coloco. Una herramienta que coloca algo con mas pasos
    // (la entrada a la mina) cambia solo esto.
    virtual bool place(stardew::maps::ElementType type, const stardew::maps::MapCell& cell);

    MapEditing& editing;

private:
    void showGhost(const std::optional<Ghost>& ghost);
    void finish();

    std::optional<stardew::maps::ElementType> type;
    std::function<void(const std::optional<Ghost>&)> ghostChanged;
    std::function<void()> done;
};
