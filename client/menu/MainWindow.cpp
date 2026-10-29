#include "client/menu/MainWindow.h"

#include <QStackedWidget>

#include "client/menu/backend/ILobbyProtocol.h"
#include "client/menu/screens/CharacterScreen.h"
#include "client/menu/screens/JoinGameScreen.h"
#include "client/menu/screens/LoginScreen.h"
#include "client/menu/screens/MainMenuScreen.h"
#include "client/menu/screens/MyGamesScreen.h"
#include "client/menu/screens/NewGameScreen.h"
#include "client/menu/screens/PlaceholderScreen.h"
#include "client/menu/styles/StyleManager.h"

MainWindow::MainWindow(ILobbyProtocol& lobby, QWidget* parent):
        QMainWindow(parent), lobby(lobby), stack(new QStackedWidget(this)), screenManager(*stack) {
    setWindowTitle("Stardew Valley");
    resize(Theme::WINDOW_WIDTH, Theme::WINDOW_HEIGHT);
    setStyleSheet(StyleManager::globalStyleSheet());
    setCentralWidget(stack);

    setupScreens();
    screenManager.show(ScreenId::Login);
}

void MainWindow::setupScreens() {
    auto* login = new LoginScreen(lobby);
    auto* mainMenu = new MainMenuScreen();
    auto* newGame = new NewGameScreen(lobby);
    auto* joinGame = new JoinGameScreen(lobby);
    auto* myGames = new MyGamesScreen(lobby);
    character = new CharacterScreen();
    // Provisoria: representa "aca arranca SDL" hasta que exista ClientApp.
    auto* inGame = new PlaceholderScreen("Partida (arranca SDL)");

    screenManager.add(ScreenId::Login, login);
    screenManager.add(ScreenId::MainMenu, mainMenu);
    screenManager.add(ScreenId::NewGame, newGame);
    screenManager.add(ScreenId::JoinGame, joinGame);
    screenManager.add(ScreenId::MyGames, myGames);
    screenManager.add(ScreenId::Character, character);
    screenManager.add(ScreenId::InGame, inGame);

    connect(login, &LoginScreen::loggedIn, this,
            [this]() { screenManager.show(ScreenId::MainMenu); });

    connect(mainMenu, &MainMenuScreen::newGameRequested, this,
            [this]() { screenManager.show(ScreenId::NewGame); });
    connect(mainMenu, &MainMenuScreen::joinGameRequested, this,
            [this]() { screenManager.show(ScreenId::JoinGame); });
    connect(mainMenu, &MainMenuScreen::myGamesRequested, this,
            [this]() { screenManager.show(ScreenId::MyGames); });
    // Cerrar la ultima ventana termina app.exec(); es la misma salida que usara IMenu::run().
    connect(mainMenu, &MainMenuScreen::exitRequested, this, &QMainWindow::close);

    // Flujo crear: nombre y mundo -> personaje -> createGame.
    connect(newGame, &NewGameScreen::gameConfigured, this,
            [this](const QString& gameName, const QString& worldId) {
                flow = CharacterFlow::Create;
                pendingCreate.gameName = gameName.toStdString();
                pendingCreate.worldId = worldId.toStdString();
                screenManager.show(ScreenId::Character);
            });

    // Flujo unirse: codigo valido -> personaje -> joinGame. Si ya es miembro no hace falta
    // personaje. Cuando exista loadGame en los DTOs, aca se llama antes de entrar.
    connect(joinGame, &JoinGameScreen::inviteCodeAccepted, this,
            [this](const CheckInviteCodeResponse& response) {
                if (response.alreadyMember) {
                    screenManager.show(ScreenId::InGame);
                    return;
                }
                flow = CharacterFlow::Join;
                pendingJoinGameId = response.gameId;
                screenManager.show(ScreenId::Character);
            });

    connect(character, &CharacterScreen::characterChosen, this, &MainWindow::submitCharacter);
    // "Volver" desde personaje regresa al paso anterior del flujo en curso.
    connect(character, &BaseScreen::backRequested, this, [this]() {
        screenManager.show(flow == CharacterFlow::Create ? ScreenId::NewGame : ScreenId::JoinGame);
    });

    // Flujo retomar: partida cargada -> juego. Cuando exista ClientApp, aca se guarda la
    // partida elegida y se cierra la ventana para que arranque SDL.
    connect(myGames, &MyGamesScreen::gameEntered, this,
            [this]() { screenManager.show(ScreenId::InGame); });

    for (BaseScreen* screen:
         {static_cast<BaseScreen*>(joinGame), static_cast<BaseScreen*>(newGame),
          static_cast<BaseScreen*>(myGames), static_cast<BaseScreen*>(inGame)}) {
        connect(screen, &BaseScreen::backRequested, this,
                [this]() { screenManager.show(ScreenId::MainMenu); });
    }
}

void MainWindow::submitCharacter(const QString& name, const CharacterAppearance& appearance) {
    LobbyStatus status = LobbyStatus::Ok;
    if (flow == CharacterFlow::Create) {
        pendingCreate.characterName = name.toStdString();
        pendingCreate.appearance = appearance;
        status = lobby.createGame(pendingCreate).status;
    } else {
        status = lobby.joinGame({pendingJoinGameId, name.toStdString(), appearance}).status;
    }

    if (status == LobbyStatus::Ok) {
        screenManager.show(ScreenId::InGame);
    } else {
        character->showError(status);
    }
}
