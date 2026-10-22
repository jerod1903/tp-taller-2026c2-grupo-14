#include <QApplication>

#include "client/menu/MainWindow.h"
#include "client/menu/backend/MockLobbyProtocol.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Provisorio: servidor falso hasta que exista ClientProtocol.
    MockLobbyProtocol lobby;
    MainWindow window(lobby);
    window.show();

    return app.exec();
}
