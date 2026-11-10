#pragma once

#include <functional>
#include <optional>
#include <string>

#include "common/map/MapDefinition.h"
#include "editor/tools/Ghost.h"
#include "editor/tools/Tool.h"

class MapEditing;

// La herramienta de areas. Un click en un area la selecciona (en una casilla vacia, se deja de
// seleccionar) y Borrar la quita con todo lo que tiene. Arrastrar desde cualquier casilla de un
// area la mueve (los pisos de la mina se mueven juntos): mientras se arrastra avisa el fantasma de
// las areas, rojo si pisan otra. Al soltar, si el movimiento separa pasajes, pregunta antes: esos
// pasajes se borran.
class AreaTool: public Tool {
public:
    explicit AreaTool(MapEditing& editing);
    void onPress(const stardew::maps::MapCell& cell) override;
    void onMove(const stardew::maps::MapCell& cell) override;
    void onRelease(const stardew::maps::MapCell& cell) override;
    void onCancel() override;
    // Deja de haber un area seleccionada
    void clear();
    // El id del area seleccionada, o vacio si no hay ninguna
    const std::optional<std::string>& selected() const;
    // Quita el area seleccionada con removeArea (quien llama ya pidio confirmacion)
    void removeSelected();
    // Cambio el area seleccionada: su id, o vacio si no hay ninguna
    void onSelectionChanged(std::function<void(const std::optional<std::string>&)> callback);
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

    void select(std::optional<std::string> areaId);
    // La respuesta a si se mueve igual; sin nadie a quien preguntar, si
    bool confirmed(int brokenPassages) const;
    void showGhost(const std::optional<AreaGhost>& ghost);
    // Termina el arrastre y saca el fantasma
    void stop();

    MapEditing& editing;
    std::optional<std::string> selectedArea;
    std::optional<Drag> drag;
    std::function<void(const std::optional<std::string>&)> selectionChanged;
    std::function<void(const std::optional<AreaGhost>&)> ghostChanged;
    std::function<bool(int)> confirmBrokenPassages;
};
