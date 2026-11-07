#include <QApplication>

#include "client/menu/MainWindow.h"
#include "client/menu/backend/MockLobbyProtocol.h"

// taller_menu: solo el menu, contra un servidor falso. Al entrar a una partida vuelve al menu
// principal, porque no hay juego.
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MockLobbyProtocol lobby;
    MainWindow window(lobby);
    window.show();

    return app.exec();
}
