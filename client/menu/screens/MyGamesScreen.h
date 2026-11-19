#pragma once

#include <QImage>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
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
    // El recuadro de la vista previa recién tiene su tamaño final después de acomodarse la
    // pantalla: cuando cambia, se vuelve a achicar el mapa
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget* createDetailPanel();
    void reloadGames();
    void showDetail(int row);
    void playSelected();
    // pide el codigo de la partida elegida y lo muestra
    void inviteSelected();
    // El mapa del mundo, achicado para entrar en el recuadro sin deformarse
    void showWorldPreview(const std::string& worldId);
    void redrawWorldPreview();

    ILobbyProtocol& lobby;
    std::vector<SavedGameInfo> games;
    // La vista previa de cada mundo, por su id: se lee su mapa una sola vez. Vacía si no se pudo.
    std::map<std::string, std::optional<QImage>> worldImages;
    // El mundo de la partida elegida
    std::string shownWorldId;

    QStackedWidget* listOrEmpty;
    QListWidget* gameList;
    QStackedWidget* detailOrHint;
    QLabel* worldPreview;
    CharacterPreview* detailCharacter;
    QLabel* detailGameName;
    QLabel* detailInfo;
    QLabel* errorLabel;
    QLabel* inviteCodeLabel;
    MenuButton* inviteButton;
    MenuButton* playButton;
};
