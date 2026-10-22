#include "client/menu/MainWindow.h"

#include <QStackedWidget>

#include "client/menu/screens/MainMenuScreen.h"
#include "client/menu/screens/PlaceholderScreen.h"
#include "client/menu/styles/StyleManager.h"

MainWindow::MainWindow(QWidget* parent):
        QMainWindow(parent), stack(new QStackedWidget(this)), screenManager(*stack) {
    setWindowTitle("Stardew Valley");
    resize(Theme::WINDOW_WIDTH, Theme::WINDOW_HEIGHT);
    setStyleSheet(StyleManager::globalStyleSheet());
    setCentralWidget(stack);

    setupScreens();
    screenManager.show(ScreenId::MainMenu);
}

void MainWindow::setupScreens() {
    auto* mainMenu = new MainMenuScreen();
    // Provisorias: se reemplazan por la pantalla real a medida que se implementan.
    auto* newGame = new PlaceholderScreen("Nueva partida");
    auto* joinGame = new PlaceholderScreen("Unirse a partida");
    auto* myGames = new PlaceholderScreen("Mis partidas");

    screenManager.add(ScreenId::MainMenu, mainMenu);
    screenManager.add(ScreenId::NewGame, newGame);
    screenManager.add(ScreenId::JoinGame, joinGame);
    screenManager.add(ScreenId::MyGames, myGames);

    connect(mainMenu, &MainMenuScreen::newGameRequested, this,
            [this]() { screenManager.show(ScreenId::NewGame); });
    connect(mainMenu, &MainMenuScreen::joinGameRequested, this,
            [this]() { screenManager.show(ScreenId::JoinGame); });
    connect(mainMenu, &MainMenuScreen::myGamesRequested, this,
            [this]() { screenManager.show(ScreenId::MyGames); });
    // Cerrar la ultima ventana termina app.exec(); es la misma salida que usara IMenu::run().
    connect(mainMenu, &MainMenuScreen::exitRequested, this, &QMainWindow::close);

    for (auto* screen: {newGame, joinGame, myGames}) {
        connect(screen, &BaseScreen::backRequested, this,
                [this]() { screenManager.show(ScreenId::MainMenu); });
    }
}
