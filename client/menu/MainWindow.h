#pragma once

#include <QMainWindow>

#include "client/menu/navigation/ScreenManager.h"

class QStackedWidget;

// Ventana del menu previo a la partida. Cada pantalla es una pagina del QStackedWidget.
class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    // Crea las pantallas y conecta sus senales con la navegacion.
    void setupScreens();

    QStackedWidget* stack;
    ScreenManager screenManager;
};
