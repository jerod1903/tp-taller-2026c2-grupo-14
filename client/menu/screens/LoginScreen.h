#pragma once

#include "client/menu/screens/BaseScreen.h"
#include "common/lobby_dto.h"

class ILobbyProtocol;
class QLabel;
class QLineEdit;
class QPushButton;
class QShowEvent;

// Primera pantalla: pide el nombre de usuario y hace el login. El nombre es la identidad
// del jugador: con el mismo nombre el servidor devuelve las mismas partidas.
class LoginScreen: public BaseScreen {
    Q_OBJECT

public:
    explicit LoginScreen(ILobbyProtocol& lobby, QWidget* parent = nullptr);

signals:
    void loggedIn(const QString& username);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void submit();
    void showError(LobbyStatus status);

    ILobbyProtocol& lobby;
    QLineEdit* nameInput;
    QLabel* errorLabel;
    QPushButton* enterButton;
};
