#pragma once

#include <QWidget>
#include <string>
#include <vector>

#include "common/map/MapDefinition.h"
#include "common/map/MapValidationIssue.h"
#include "common/map/service/MapService.h"
#include "editor/MapEditing.h"
#include "editor/SpriteSheets.h"
#include "editor/tools/PaintTool.h"
#include "editor/tools/PassageTool.h"

class MapCanvas;
class QHBoxLayout;

// Pantalla de edicion: el mapa entero, pintar con el pincel, validar y guardar.
class MapEditorScreen: public QWidget {
    Q_OBJECT

public:
    explicit MapEditorScreen(const stardew::maps::MapService& service, QWidget* parent = nullptr);
    // Lee el mapa y lo muestra. Si tiene errores igual se abre, para poder corregirlo.
    void open(const std::string& path);

signals:
    void backRequested();

private:
    QHBoxLayout* createPalette();
    void validate();
    void save();
    void showIssues(const QString& title,
                    const std::vector<stardew::maps::MapValidationIssue>& issues);

    const stardew::maps::MapService& service;
    // El mapa vive en editing; las herramientas lo cambian a traves de el y el lienzo lo dibuja
    MapEditing editing;
    PaintTool paintTool{editing};
    PassageTool passageTool{editing};
    std::string path;
    SpriteSheets sprites;
    MapCanvas* canvas;
};
