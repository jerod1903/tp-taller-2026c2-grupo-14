#pragma once

#include "client/menu/backend/LobbyTypes.h"
#include "client/menu/screens/BaseScreen.h"

class ILobbyProtocol;
class QLabel;
class QLineEdit;
class QPushButton;
class QShowEvent;

// Pide el codigo de invitacion y lo verifica con el servidor antes de seguir.
class JoinGameScreen: public BaseScreen {
    Q_OBJECT

public:
    explicit JoinGameScreen(ILobbyProtocol& lobby, QWidget* parent = nullptr);

signals:
    // El servidor acepto el codigo: el flujo sigue en la pantalla del personaje.
    void inviteCodeAccepted(const QString& code);

protected:
    // Cada vez que se entra a la pantalla arranca limpia.
    void showEvent(QShowEvent* event) override;

private:
    void submitCode();
    void showError(LobbyStatus status);

    ILobbyProtocol& lobby;
    QLineEdit* codeInput;
    QLabel* errorLabel;
    QPushButton* continueButton;
};
