#pragma once

#include <cstdint>
#include <vector>

#include "client/menu/screens/BaseScreen.h"
#include "common/lobby_dto.h"

class CharacterPreview;
class ILobbyProtocol;
class QLabel;
class QListWidget;
class MenuButton;
class QShowEvent;
class QStackedWidget;

// "Retomar partida": lista las partidas del jugador (vertical, una fila por partida)
// y a la izquierda muestra el detalle de la seleccionada.
class MyGamesScreen: public BaseScreen {
    Q_OBJECT

public:
    explicit MyGamesScreen(ILobbyProtocol& lobby, QWidget* parent = nullptr);

signals:
    // El servidor cargo la partida: el menu termina y empieza el juego.
    void gameEntered(const GameEnteredResponse& response);

protected:
    // La lista se pide al servidor cada vez que se entra, para que este actualizada.
    void showEvent(QShowEvent* event) override;

private:
    QWidget* createDetailPanel();
    void reloadGames();
    void showDetail(int row);
    void playSelected();

    ILobbyProtocol& lobby;
    std::vector<SavedGameInfo> games;

    QStackedWidget* listOrEmpty;
    QListWidget* gameList;
    QStackedWidget* detailOrHint;
    QLabel* worldPreview;
    CharacterPreview* detailCharacter;
    QLabel* detailGameName;
    QLabel* detailInfo;
    QLabel* errorLabel;
    MenuButton* playButton;
};
