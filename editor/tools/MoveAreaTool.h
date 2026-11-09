#pragma once

#include <functional>
#include <optional>
#include <string>

#include "common/map/MapDefinition.h"
#include "editor/tools/Ghost.h"
#include "editor/tools/Tool.h"

class MapEditing;

// Arrastrar desde cualquier casilla de un area la mueve con todo lo que tiene (los pisos de la
// mina se mueven juntos). Mientras se arrastra avisa el fantasma de las areas, rojo si pisan otra.
// Al soltar, si el movimiento separa pasajes, pregunta antes: esos pasajes se borran.
class MoveAreaTool: public Tool {
public:
    explicit MoveAreaTool(MapEditing& editing);
    void onPress(const stardew::maps::MapCell& cell) override;
    void onMove(const stardew::maps::MapCell& cell) override;
    void onRelease(const stardew::maps::MapCell& cell) override;
    void onCancel() override;
    // Cambio el fantasma: vacio si no hay que mostrarlo
    void onGhostChanged(std::function<void(const std::optional<AreaGhost>&)> callback);
    // Pregunta si se mueve igual aunque se borren esos pasajes; sin respuesta, se mueve
    void onConfirmBrokenPassages(std::function<bool(int brokenPassages)> callback);

private:
    // El area que se esta arrastrando y desde que casilla
    struct Drag {
        std::string areaId;
        stardew::maps::MapCell start;
        int dx;
        int dy;
    };

    // La respuesta a si se mueve igual; sin nadie a quien preguntar, si
    bool confirmed(int brokenPassages) const;
    void showGhost(const std::optional<AreaGhost>& ghost);
    // Termina el arrastre y saca el fantasma
    void stop();

    MapEditing& editing;
    std::optional<Drag> drag;
    std::function<void(const std::optional<AreaGhost>&)> ghostChanged;
    std::function<bool(int)> confirmBrokenPassages;
};
