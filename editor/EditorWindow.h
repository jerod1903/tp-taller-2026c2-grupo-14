#pragma once

#include <QMainWindow>

#include "common/map/persistence/MapRepository.h"
#include "common/map/service/MapService.h"

class MapEditorScreen;
class QStackedWidget;
class QWidget;

// Ventana del editor de mapas: el menu de inicio y la pantalla de edicion, con el estilo del menu
// del juego.
class EditorWindow: public QMainWindow {
public:
    explicit EditorWindow(QWidget* parent = nullptr);

private:
    QWidget* createMenu();
    void openMap();

    stardew::maps::TomlMapRepository repository;
    stardew::maps::MapService service;
    QStackedWidget* stack;
    QWidget* menu;
    MapEditorScreen* editor;
};
