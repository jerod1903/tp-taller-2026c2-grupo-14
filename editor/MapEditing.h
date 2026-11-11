#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"
#include "editor/MapHelper.h"
#include "editor/Mine.h"

// Cuanto ocuparia un elemento apoyado en una casilla y si entra ahi
struct Placement {
    int width;
    int height;
    bool fits;
};

// Donde quedarian las areas que se mueven juntas, si entran sin pisar otra y cuantos pasajes
// quedarian separados (se borran al mover)
struct AreaMove {
    std::vector<CellRect> areas;
    bool fits;
    int brokenPassages;
};

// Lo que se iria al borrar un area: las areas (un piso se lleva todos los pisos) y cuantos
// elementos tienen
struct AreaRemoval {
    std::vector<std::string> areaIds;
    int elements;
};

// El mapa que se esta editando y las operaciones que lo cambian. No sabe nada de Qt: quien lo
// muestra se entera de los cambios con los callbacks.
class MapEditing {
public:
    // De las reglas solo se usan el tamano y la capa de los elementos que se colocan: las reglas se
    // revisan al guardar
    explicit MapEditing(const stardew::maps::MapRequirements& requirements);

    // Reemplaza el mapa (al abrir o crear uno) y avisa que cambio todo
    void load(stardew::maps::MapDefinition map);
    const stardew::maps::MapDefinition& map() const;
    // Si cambio algo desde que se cargo o se guardo
    bool hasUnsavedChanges() const;
    void markSaved();

    // Las operaciones reciben casillas del mapa general
    void paint(const stardew::maps::MapCell& cell, stardew::maps::TileType tile);
    void togglePassage(const stardew::maps::MapCell& cell);
    // Quita el elemento y sus transiciones. Un porton se va con su par; la salida y la escalera de
    // los pisos no se borran. Si se borra una entrada a la mina y queda otra, las salidas pasan a
    // llevar frente a esa.
    void removeElement(const std::string& id);
    // Lleva el elemento a otra casilla (esquina de abajo a la izquierda de su base). Los portones
    // no se mueven. La salida y la escalera se mueven en todos los pisos, y si se mueve una
    // entrada, las salidas pasan a llevar frente a ella. Devuelve false si no entra.
    bool moveElement(const std::string& id, const stardew::maps::MapCell& bottomLeft);
    // Lo que ocupa el elemento y si moveElement lo llevaria a esa casilla. Vacio si no se mueve
    // (un porton o un id que no existe).
    std::optional<Placement> movementOf(const std::string& id,
                                        const stardew::maps::MapCell& bottomLeft) const;
    // Corre (dx, dy) el area con sus elementos; los pisos de la mina se corren todos juntos. Los
    // pasajes que quedan separados se borran. Devuelve false si pisaria otra area.
    bool moveArea(const std::string& areaId, int dx, int dy);
    // Donde quedarian las areas con ese corrimiento. Vacio si no hay area con ese id.
    std::optional<AreaMove> areaMoveOf(const std::string& areaId, int dx, int dy) const;
    // Agrega una granja o un pueblo vacio con su esquina de arriba a la izquierda en la casilla.
    // Devuelve false si pisaria otra area.
    bool placeArea(stardew::maps::AreaType type, const AreaSize& size,
                   const stardew::maps::MapCell& topLeft);
    // Donde quedaria un area de ese tamano y si entra sin pisar otra
    AreaMove areaPlacementOf(const AreaSize& size, const stardew::maps::MapCell& topLeft) const;
    // Quita el area con sus elementos y todas las transiciones que la tocan; un piso se lleva
    // todos los pisos. Si se va una entrada a la mina y queda otra, las salidas pasan a esa.
    void removeArea(const std::string& areaId);
    // Lo que se iria con removeArea. Vacio si no hay area con ese id.
    std::optional<AreaRemoval> areaRemovalOf(const std::string& areaId) const;
    // Pone un elemento de ese tipo con la esquina de abajo a la izquierda de su base en la casilla.
    // Devuelve false si la base no cae entera en una sola area o pisa otro elemento.
    bool placeElement(stardew::maps::ElementType type, const stardew::maps::MapCell& bottomLeft);
    // Pone una entrada a la mina como placeElement y la une con el primer piso. Si el mapa todavia
    // no tiene pisos, los crea de ese tamano. Si es la unica entrada, las salidas de los pisos
    // pasan a llevar frente a ella.
    bool placeEntrance(const stardew::maps::MapCell& bottomLeft,
                       const std::optional<AreaSize>& floorSize);
    // Lo que ocuparia un elemento de ese tipo apoyado en la casilla y si placeElement lo pondria.
    // Vacio si no hay regla para ese tipo.
    std::optional<Placement> placementOf(stardew::maps::ElementType type,
                                         const stardew::maps::MapCell& bottomLeft) const;

    // Cambio el terreno de una casilla
    void onTileChanged(std::function<void(const stardew::maps::MapCell&)> callback);
    // Cambio algo mas que una casilla (elementos, transiciones, el mapa entero)
    void onMapChanged(std::function<void()> callback);

private:
    // Marca que hay cambios sin guardar y avisa que cambio el mapa
    void notifyMapChanged();

    const stardew::maps::MapRequirements& requirements;
    stardew::maps::MapDefinition definition;
    bool unsaved = false;
    std::function<void(const stardew::maps::MapCell&)> tileChanged;
    std::function<void()> mapChanged;
};
