#pragma once

#include <QApplication>
#include <optional>

#include "client/app/IMenu.h"
#include "client/menu/MainWindow.h"

class ILobbyProtocol;

// El menu en Qt. La QApplication y la ventana se crean una sola vez y sobreviven a las partidas:
// al volver se mantienen el usuario logueado y las pantallas.
class QtMenu: public IMenu {
public:
    // argc tiene que vivir mientras viva el menu (lo pide QApplication).
    QtMenu(int& argc, char** argv, ILobbyProtocol& lobby);
    std::optional<GameSession> run() override;

private:
    QApplication app;
    MainWindow window;
    std::optional<GameSession> entered;
};
