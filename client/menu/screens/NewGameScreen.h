#pragma once

#include <vector>

#include "client/menu/screens/BaseScreen.h"
#include "common/lobby_dto.h"

class ILobbyProtocol;
class QLineEdit;
class QListWidget;
class QPushButton;
class QShowEvent;
class QStackedWidget;

// Primer paso de "Nueva partida": nombre de la partida y mundo. La partida se crea
// recien despues de elegir el personaje, asi que esta pantalla solo emite lo elegido.
// Las partidas se crean siempre individuales; pasarlas a multijugador es despues.
class NewGameScreen: public BaseScreen {
    Q_OBJECT

public:
    explicit NewGameScreen(ILobbyProtocol& lobby, QWidget* parent = nullptr);

signals:
    void gameConfigured(const QString& gameName, const QString& worldId);

protected:
    // La lista de mundos se pide al servidor cada vez que se entra.
    void showEvent(QShowEvent* event) override;

private:
    void reloadWorlds();
    void refreshContinueButton();
    void submit();

    ILobbyProtocol& lobby;
    std::vector<WorldInfo> worlds;

    QLineEdit* nameInput;
    QStackedWidget* listOrEmpty;
    QListWidget* worldList;
    QPushButton* continueButton;
};
