#include "client/app/ClientApp.h"

#include <optional>

#include "client/app/IGameView.h"
#include "client/app/IMenu.h"

ClientApp::ClientApp(IMenu& menu, IGameView& game): menu(menu), game(game) {}

void ClientApp::run() {
    while (std::optional<GameSession> session = menu.run()) {
        game.run(*session);
    }
}
