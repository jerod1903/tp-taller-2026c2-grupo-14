#include "client/menu/QtMenu.h"

QtMenu::QtMenu(int& argc, char** argv, ILobbyProtocol& lobby): app(argc, argv), window(lobby) {
    // Al entrar a una partida se esconde la ventana y termina exec(): run() devuelve la partida.
    QObject::connect(&window, &MainWindow::gameEntered, &window,
                     [this](const GameSession& session) {
                         entered = session;
                         window.hide();
                         QApplication::quit();
                     });
}

std::optional<GameSession> QtMenu::run() {
    entered.reset();
    window.show();
    // Si el jugador cierra la ventana, exec() vuelve sin partida.
    QApplication::exec();
    return entered;
}
