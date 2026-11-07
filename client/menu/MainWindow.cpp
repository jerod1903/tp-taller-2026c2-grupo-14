#include "client/menu/MainWindow.h"

#include <QPalette>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QStringList>

#include "client/menu/backend/ILobbyProtocol.h"
#include "client/menu/screens/CharacterScreen.h"
#include "client/menu/screens/JoinGameScreen.h"
#include "client/menu/screens/LoginScreen.h"
#include "client/menu/screens/MainMenuScreen.h"
#include "client/menu/screens/MyGamesScreen.h"
#include "client/menu/screens/NewGameScreen.h"
#include "client/menu/styles/StyleManager.h"

namespace {
const QStringList BACKGROUNDS = {":/menu/fondos/tienda.png", ":/menu/fondos/casa_morada.png",
                                 ":/menu/fondos/casa_roja.png"};
}  // namespace

MainWindow::MainWindow(ILobbyProtocol& lobby, QWidget* parent):
        QMainWindow(parent), lobby(lobby), stack(new QStackedWidget(this)), screenManager(*stack) {
    setWindowTitle("Stardew Valley");
    // Tamano fijo: los sprites de madera se agrandan con repeticion, no con escalado.
    setFixedSize(Theme::WINDOW_WIDTH, Theme::WINDOW_HEIGHT);
    setStyleSheet(StyleManager::globalStyleSheet());
    setCentralWidget(stack);

    // El stack pinta el fondo; las pantallas encima no tienen fondo propio.
    stack->setAutoFillBackground(true);
    int index = QRandomGenerator::global()->bounded(BACKGROUNDS.size());
    background = QPixmap(BACKGROUNDS.at(index));

    setupScreens();
    screenManager.show(ScreenId::Login);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateBackground();
}

void MainWindow::updateBackground() {
    if (background.isNull() || stack->size().isEmpty()) {
        return;
    }
    // Cubre toda la ventana y recorta el centro. FastTransformation mantiene el pixel art nitido.
    QSize target = stack->size();
    QPixmap scaled =
            background.scaled(target, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    QPixmap cropped =
            scaled.copy((scaled.width() - target.width()) / 2,
                        (scaled.height() - target.height()) / 2, target.width(), target.height());
    QPalette palette = stack->palette();
    palette.setBrush(QPalette::Window, cropped);
    stack->setPalette(palette);
}

void MainWindow::setupScreens() {
    auto* login = new LoginScreen(lobby);
    auto* mainMenu = new MainMenuScreen();
    auto* newGame = new NewGameScreen(lobby);
    auto* joinGame = new JoinGameScreen(lobby);
    auto* myGames = new MyGamesScreen(lobby);
    character = new CharacterScreen();

    screenManager.add(ScreenId::Login, login);
    screenManager.add(ScreenId::MainMenu, mainMenu);
    screenManager.add(ScreenId::NewGame, newGame);
    screenManager.add(ScreenId::JoinGame, joinGame);
    screenManager.add(ScreenId::MyGames, myGames);
    screenManager.add(ScreenId::Character, character);

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

    // Flujo unirse: codigo valido -> personaje -> joinGame.
    connect(joinGame, &JoinGameScreen::inviteCodeAccepted, this, [this](const QString& code) {
        flow = CharacterFlow::Join;
        pendingInviteCode = code.toStdString();
        screenManager.show(ScreenId::Character);
    });

    connect(character, &CharacterScreen::characterChosen, this, &MainWindow::submitCharacter);
    // "Volver" desde personaje regresa al paso anterior del flujo en curso.
    connect(character, &BaseScreen::backRequested, this, [this]() {
        screenManager.show(flow == CharacterFlow::Create ? ScreenId::NewGame : ScreenId::JoinGame);
    });

    // Flujo retomar: partida cargada -> juego.
    connect(myGames, &MyGamesScreen::gameEntered, this, &MainWindow::enterGame);

    for (BaseScreen* screen: {static_cast<BaseScreen*>(joinGame), static_cast<BaseScreen*>(newGame),
                              static_cast<BaseScreen*>(myGames)}) {
        connect(screen, &BaseScreen::backRequested, this,
                [this]() { screenManager.show(ScreenId::MainMenu); });
    }
}

void MainWindow::submitCharacter(const QString& name, const CharacterAppearance& appearance) {
    GameEnteredResponse response{};
    if (flow == CharacterFlow::Create) {
        pendingCreate.characterName = name.toStdString();
        pendingCreate.appearance = appearance;
        response = lobby.createGame(pendingCreate);
    } else {
        response = lobby.joinGame({pendingInviteCode, name.toStdString(), appearance});
    }

    if (response.status == LobbyStatus::Ok) {
        enterGame(response);
    } else {
        character->showError(response.status);
    }
}

void MainWindow::enterGame(const GameEnteredResponse& response) {
    screenManager.show(ScreenId::MainMenu);
    emit gameEntered({response.gameId, response.playerId});
}
