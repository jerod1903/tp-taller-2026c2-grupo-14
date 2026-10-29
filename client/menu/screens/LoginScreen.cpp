#include "client/menu/screens/LoginScreen.h"

#include <QLabel>
#include <QLineEdit>
#include <QShowEvent>
#include <QVBoxLayout>

#include "client/menu/backend/ILobbyProtocol.h"
#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"

namespace {
// El mismo limite que aplica el servidor a los nombres.
constexpr int MAX_NAME_LENGTH = 20;
}  // namespace

LoginScreen::LoginScreen(ILobbyProtocol& lobby, QWidget* parent):
        BaseScreen(parent),
        lobby(lobby),
        nameInput(new QLineEdit(this)),
        errorLabel(new QLabel(this)),
        enterButton(new MenuButton("Entrar", this)) {
    nameInput->setPlaceholderText("Ej: martu");
    nameInput->setMaxLength(MAX_NAME_LENGTH);

    errorLabel->setObjectName("errorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    // El espacio queda reservado aunque no haya error: mostrar uno no mueve nada.
    errorLabel->setFixedHeight(Theme::ERROR_LABEL_HEIGHT);

    // Sin nombre no hay con que entrar.
    enterButton->setEnabled(false);

    auto* formColumn = new QWidget(this);
    formColumn->setFixedWidth(Theme::MENU_COLUMN_WIDTH);
    auto* formLayout = new QVBoxLayout(formColumn);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(Theme::SCREEN_SPACING);
    formLayout->addWidget(new QLabel("Nombre de usuario", formColumn));
    formLayout->addWidget(nameInput);
    formLayout->addWidget(errorLabel);
    formLayout->addWidget(enterButton);

    // Sin "Volver": es la primera pantalla.
    mainLayout->addStretch();
    mainLayout->addWidget(createTitle("Stardew Valley"));
    mainLayout->addSpacing(30);
    mainLayout->addWidget(formColumn, 0, Qt::AlignHCenter);
    mainLayout->addStretch();

    connect(nameInput, &QLineEdit::textChanged, this, [this](const QString& text) {
        enterButton->setEnabled(!text.trimmed().isEmpty());
        errorLabel->clear();
    });
    connect(nameInput, &QLineEdit::returnPressed, this, &LoginScreen::submit);
    connect(enterButton, &QPushButton::clicked, this, &LoginScreen::submit);
}

void LoginScreen::showEvent(QShowEvent* event) {
    BaseScreen::showEvent(event);
    nameInput->setFocus();
}

void LoginScreen::submit() {
    const QString username = nameInput->text().trimmed();
    if (username.isEmpty()) {
        return;
    }

    LobbyStatus status = lobby.login(username.toStdString());
    if (status == LobbyStatus::Ok) {
        emit loggedIn(username);
    } else {
        showError(status);
    }
}

void LoginScreen::showError(LobbyStatus status) {
    switch (status) {
        case LobbyStatus::InvalidName:
            errorLabel->setText("Usá solo letras, números, guion o guion bajo, sin espacios.");
            break;
        case LobbyStatus::AlreadyConnected:
            errorLabel->setText("Ese usuario ya está conectado.");
            break;
        case LobbyStatus::ProtocolMismatch:
            errorLabel->setText("Esta versión del juego no es compatible con el servidor.");
            break;
        default:
            errorLabel->setText("No se pudo entrar. Intentá de nuevo.");
            break;
    }
}
