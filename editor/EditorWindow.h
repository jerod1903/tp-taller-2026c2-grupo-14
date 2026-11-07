#pragma once

#include <QMainWindow>

// Ventana del editor de mapas. Por ahora solo el menu de inicio, con el estilo del menu del juego.
class EditorWindow: public QMainWindow {
public:
    explicit EditorWindow(QWidget* parent = nullptr);
};
