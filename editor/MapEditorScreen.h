#pragma once

#include <QWidget>
#include <string>
#include <vector>

#include "common/map/MapDefinition.h"
#include "common/map/MapValidationIssue.h"
#include "common/map/service/MapService.h"
#include "editor/MapEditing.h"
#include "editor/SpriteSheets.h"
#include "editor/tools/AreaTool.h"
#include "editor/tools/EntranceTool.h"
#include "editor/tools/PaintTool.h"
#include "editor/tools/PassageTool.h"
#include "editor/tools/PlaceAreaTool.h"
#include "editor/tools/PlaceTool.h"
#include "editor/tools/SelectTool.h"

class MapCanvas;
class QButtonGroup;
class QHBoxLayout;
class QToolButton;
class Tool;

// Pantalla de edicion: el mapa entero, pintar con el pincel, poner pasajes, colocar elementos (la
// entrada a la mina crea los pisos), seleccionar, mover y borrar elementos, crear, mover y borrar
// areas, validar y guardar.
class MapEditorScreen: public QWidget {
    Q_OBJECT

public:
    explicit MapEditorScreen(const stardew::maps::MapService& service, QWidget* parent = nullptr);
    // Lee el mapa y lo muestra. Si tiene errores igual se abre, para poder corregirlo.
    void open(const std::string& path);

signals:
    void backRequested();

private:
    // Las herramientas de arriba; sus botones entran al grupo junto con las tarjetas del panel
    QHBoxLayout* createPalette(QButtonGroup* group);
    // Cambia la herramienta del lienzo; lo que estaba seleccionado deja de estarlo y el fantasma
    // del elemento por colocar se saca
    void useTool(Tool* tool);
    // Une el fantasma de una herramienta de colocar con el lienzo; al terminar se vuelve a
    // Seleccionar
    void connectPlacing(PlaceTool& tool);
    // Si el mapa no tiene pisos pide su tamano; cancelar vuelve a Seleccionar
    void chooseEntrance();
    // Pide el tamano de la granja o el pueblo nuevo; cancelar vuelve a Seleccionar
    void chooseArea(stardew::maps::AreaType type);
    // Marca Seleccionar y la usa
    void backToSelect();
    // Borra lo seleccionado en la herramienta activa: un elemento en Seleccionar, un area (despues
    // de confirmar) en Areas
    void removeSelected();
    void validate();
    void save();
    void showIssues(const QString& title,
                    const std::vector<stardew::maps::MapValidationIssue>& issues);

    const stardew::maps::MapService& service;
    // El mapa vive en editing; las herramientas lo cambian a traves de el y el lienzo lo dibuja
    MapEditing editing{service.requirements()};
    PaintTool paintTool{editing};
    PassageTool passageTool{editing};
    PlaceTool placeTool{editing};
    EntranceTool entranceTool{editing};
    SelectTool selectTool{editing};
    AreaTool areaTool{editing};
    PlaceAreaTool placeAreaTool{editing};
    Tool* activeTool = nullptr;
    std::string path;
    SpriteSheets sprites;
    MapCanvas* canvas;
    QToolButton* selectButton = nullptr;
    QToolButton* removeButton = nullptr;
};
