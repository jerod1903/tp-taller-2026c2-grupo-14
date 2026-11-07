#include "client/menu/screens/NewGameScreen.h"

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QShowEvent>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <optional>
#include <utility>
#include <vector>

#include "client/menu/backend/ILobbyProtocol.h"
#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"

namespace {
// El mismo limite que aplica el servidor a los nombres.
constexpr int MAX_NAME_LENGTH = 20;
constexpr int WORLD_LIST_HEIGHT = 220;
}  // namespace

NewGameScreen::NewGameScreen(ILobbyProtocol& lobby, QWidget* parent):
        BaseScreen(parent),
        lobby(lobby),
        nameInput(new QLineEdit(this)),
        listOrEmpty(new QStackedWidget(this)),
        worldList(new QListWidget(this)),
        continueButton(new MenuButton("Continuar", this)) {
    nameInput->setPlaceholderText("Ej: Mi_granja");
    nameInput->setMaxLength(MAX_NAME_LENGTH);

    // Mundos: la lista, o un mensaje si el servidor no tiene ninguno cargado.
    auto* emptyMessage = new QLabel("El servidor no tiene mundos disponibles.", this);
    emptyMessage->setAlignment(Qt::AlignCenter);
    emptyMessage->setWordWrap(true);
    worldList->setObjectName("worldList");
    listOrEmpty->addWidget(worldList);
    listOrEmpty->addWidget(emptyMessage);
    listOrEmpty->setFixedHeight(WORLD_LIST_HEIGHT);

    // Hace falta nombre y mundo para seguir.
    continueButton->setEnabled(false);

    auto* formColumn = new QWidget(this);
    formColumn->setFixedWidth(Theme::MENU_COLUMN_WIDTH);
    auto* formLayout = new QVBoxLayout(formColumn);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(Theme::SCREEN_SPACING);
    formLayout->addWidget(new QLabel("Nombre de la partida", formColumn));
    formLayout->addWidget(nameInput);
    formLayout->addWidget(new QLabel("Mundo", formColumn));
    formLayout->addWidget(listOrEmpty);
    formLayout->addWidget(continueButton);

    addBackButton();
    mainLayout->addStretch();
    setTitle("Nueva partida");
    mainLayout->addSpacing(30);
    mainLayout->addWidget(formColumn, 0, Qt::AlignHCenter);
    mainLayout->addStretch();

    connect(nameInput, &QLineEdit::textChanged, this, &NewGameScreen::refreshContinueButton);
    connect(nameInput, &QLineEdit::returnPressed, this, &NewGameScreen::submit);
    connect(worldList, &QListWidget::currentRowChanged, this,
            &NewGameScreen::refreshContinueButton);
    // Doble click en un mundo es lo mismo que elegirlo y tocar "Continuar".
    connect(worldList, &QListWidget::itemDoubleClicked, this, &NewGameScreen::submit);
    connect(continueButton, &QPushButton::clicked, this, &NewGameScreen::submit);
}

void NewGameScreen::showEvent(QShowEvent* event) {
    BaseScreen::showEvent(event);
    nameInput->clear();
    reloadWorlds();
    nameInput->setFocus();
}

void NewGameScreen::reloadWorlds() {
    std::optional<std::vector<WorldInfo>> received =
            guarded([this]() { return lobby.listWorlds(); });
    if (!received) {
        return;
    }
    worlds = std::move(*received);
    worldList->clear();
    for (const WorldInfo& world: worlds) {
        worldList->addItem(QString::fromStdString(world.name));
    }
    listOrEmpty->setCurrentIndex(worlds.empty() ? 1 : 0);
    // Arranca con el primer mundo elegido: casi siempre se juega en el mismo.
    if (!worlds.empty()) {
        worldList->setCurrentRow(0);
    }
    refreshContinueButton();
}

void NewGameScreen::refreshContinueButton() {
    const int row = worldList->currentRow();
    const bool hasWorld = row >= 0 && static_cast<std::size_t>(row) < worlds.size();
    continueButton->setEnabled(hasWorld && !nameInput->text().trimmed().isEmpty());
}

void NewGameScreen::submit() {
    if (!continueButton->isEnabled()) {
        return;
    }
    const WorldInfo& world = worlds[static_cast<std::size_t>(worldList->currentRow())];
    emit gameConfigured(nameInput->text().trimmed(), QString::fromStdString(world.worldId));
}
