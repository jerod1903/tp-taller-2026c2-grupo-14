#pragma once

#include <QMainWindow>

#include "client/menu/navigation/ScreenManager.h"

class ILobbyProtocol;
class QStackedWidget;

// Ventana del menu previo a la partida. Cada pantalla es una pagina del QStackedWidget.
class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    // lobby: acceso al servidor; MainWindow se lo pasa a las pantallas que lo necesitan.
    explicit MainWindow(ILobbyProtocol& lobby, QWidget* parent = nullptr);

private:
    // Crea las pantallas y conecta sus senales con la navegacion.
    void setupScreens();

    ILobbyProtocol& lobby;
    QStackedWidget* stack;
    ScreenManager screenManager;
};
