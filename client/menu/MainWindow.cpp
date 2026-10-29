#include "client/menu/MainWindow.h"

#include <QStackedWidget>

#include "client/menu/screens/CharacterScreen.h"
#include "client/menu/screens/JoinGameScreen.h"
#include "client/menu/screens/MainMenuScreen.h"
#include "client/menu/screens/MyGamesScreen.h"
#include "client/menu/screens/PlaceholderScreen.h"
#include "client/menu/styles/StyleManager.h"

MainWindow::MainWindow(ILobbyProtocol& lobby, QWidget* parent):
        QMainWindow(parent), lobby(lobby), stack(new QStackedWidget(this)), screenManager(*stack) {
    setWindowTitle("Stardew Valley");
    resize(Theme::WINDOW_WIDTH, Theme::WINDOW_HEIGHT);
    setStyleSheet(StyleManager::globalStyleSheet());
    setCentralWidget(stack);

    setupScreens();
    screenManager.show(ScreenId::MainMenu);
}

void MainWindow::setupScreens() {
    auto* mainMenu = new MainMenuScreen();
    auto* joinGame = new JoinGameScreen(lobby);
    auto* myGames = new MyGamesScreen(lobby);
    auto* character = new CharacterScreen();
    // Provisorias: se reemplazan por la pantalla real a medida que se implementan.
    auto* newGame = new PlaceholderScreen("Nueva partida");
    auto* inGame = new PlaceholderScreen("Partida (arranca SDL)");

    screenManager.add(ScreenId::MainMenu, mainMenu);
    screenManager.add(ScreenId::NewGame, newGame);
    screenManager.add(ScreenId::JoinGame, joinGame);
    screenManager.add(ScreenId::MyGames, myGames);
    screenManager.add(ScreenId::Character, character);
    screenManager.add(ScreenId::InGame, inGame);

    connect(mainMenu, &MainMenuScreen::newGameRequested, this,
            [this]() { screenManager.show(ScreenId::NewGame); });
    connect(mainMenu, &MainMenuScreen::joinGameRequested, this,
            [this]() { screenManager.show(ScreenId::JoinGame); });
    connect(mainMenu, &MainMenuScreen::myGamesRequested, this,
            [this]() { screenManager.show(ScreenId::MyGames); });
    // Cerrar la ultima ventana termina app.exec(); es la misma salida que usara IMenu::run().
    connect(mainMenu, &MainMenuScreen::exitRequested, this, &QMainWindow::close);

    // Flujo unirse: codigo valido -> personaje. El gameId de la respuesta lo va a necesitar
    // la pantalla de personaje para armar el JoinGameRequest; por ahora solo se navega.
    connect(joinGame, &JoinGameScreen::inviteCodeAccepted, this,
            [this]() { screenManager.show(ScreenId::Character); });
    connect(character, &BaseScreen::backRequested, this,
            [this]() { screenManager.show(ScreenId::JoinGame); });

    // Flujo retomar: partida cargada -> juego. Cuando exista QtMenu, aca se guarda la
    // GameSession y se cierra la ventana para que arranque SDL.
    connect(myGames, &MyGamesScreen::gameEntered, this,
            [this]() { screenManager.show(ScreenId::InGame); });

    for (BaseScreen* screen:
         {static_cast<BaseScreen*>(joinGame), static_cast<BaseScreen*>(newGame),
          static_cast<BaseScreen*>(myGames), static_cast<BaseScreen*>(inGame)}) {
        connect(screen, &BaseScreen::backRequested, this,
                [this]() { screenManager.show(ScreenId::MainMenu); });
    }
}
