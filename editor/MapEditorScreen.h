#pragma once

#include <QWidget>
#include <string>
#include <vector>

#include "common/map/MapDefinition.h"
#include "common/map/MapValidationIssue.h"
#include "common/map/service/MapService.h"
#include "editor/SpriteSheets.h"

class AreaCanvas;
class QComboBox;
class QHBoxLayout;

// Pantalla de edicion: elegir area, pintar con el pincel, validar y guardar.
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
    void showSelectedArea();
    void validate();
    void save();
    void showIssues(const QString& title,
                    const std::vector<stardew::maps::MapValidationIssue>& issues);

    const stardew::maps::MapService& service;
    stardew::maps::MapDefinition map;
    std::string path;
    SpriteSheets sprites;
    QComboBox* areaSelector;
    AreaCanvas* canvas;
};
