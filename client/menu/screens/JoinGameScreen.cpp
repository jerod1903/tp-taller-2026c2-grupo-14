#include "client/menu/screens/JoinGameScreen.h"

#include <QLabel>
#include <QLineEdit>
#include <QShowEvent>
#include <QVBoxLayout>

#include "client/menu/backend/ILobbyProtocol.h"
#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"

JoinGameScreen::JoinGameScreen(ILobbyProtocol& lobby, QWidget* parent):
        BaseScreen(parent),
        lobby(lobby),
        codeInput(new QLineEdit(this)),
        errorLabel(new QLabel(this)),
        continueButton(new MenuButton("Continuar", this)) {
    auto* codeLabel = new QLabel("Código de invitación", this);
    codeInput->setPlaceholderText("Ej: ABC123");

    errorLabel->setObjectName("errorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    // El espacio queda reservado aunque no haya error: mostrar uno no mueve nada.
    errorLabel->setFixedHeight(Theme::ERROR_LABEL_HEIGHT);

    // Sin codigo no hay nada que verificar.
    continueButton->setEnabled(false);

    auto* formColumn = new QWidget(this);
    formColumn->setFixedWidth(Theme::MENU_COLUMN_WIDTH);
    auto* formLayout = new QVBoxLayout(formColumn);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(Theme::SCREEN_SPACING);
    formLayout->addWidget(codeLabel);
    formLayout->addWidget(codeInput);
    formLayout->addWidget(errorLabel);
    formLayout->addWidget(continueButton);

    addBackButton();
    mainLayout->addStretch();
    mainLayout->addWidget(createTitle("Unirse a partida"));
    mainLayout->addSpacing(30);
    mainLayout->addWidget(formColumn, 0, Qt::AlignHCenter);
    mainLayout->addStretch();

    connect(codeInput, &QLineEdit::textChanged, this, [this](const QString& text) {
        continueButton->setEnabled(!text.trimmed().isEmpty());
        errorLabel->clear();
    });
    connect(codeInput, &QLineEdit::returnPressed, this, &JoinGameScreen::submitCode);
    connect(continueButton, &QPushButton::clicked, this, &JoinGameScreen::submitCode);
}

void JoinGameScreen::showEvent(QShowEvent* event) {
    BaseScreen::showEvent(event);
    codeInput->clear();
    codeInput->setFocus();
}

void JoinGameScreen::submitCode() {
    const QString code = codeInput->text().trimmed();
    if (code.isEmpty()) {
        return;
    }

    LobbyStatus status = lobby.checkInviteCode(code.toStdString());
    if (status == LobbyStatus::Ok) {
        emit inviteCodeAccepted(code);
    } else {
        showError(status);
    }
}

void JoinGameScreen::showError(LobbyStatus status) {
    switch (status) {
        case LobbyStatus::InvalidInviteCode:
            errorLabel->setText("No hay ninguna partida con ese código. Revisalo e intentá de nuevo.");
            break;
        case LobbyStatus::GameFull:
            errorLabel->setText("La partida ya tiene el máximo de jugadores.");
            break;
        default:
            errorLabel->setText("No se pudo verificar el código. Intentá de nuevo.");
            break;
    }
}
