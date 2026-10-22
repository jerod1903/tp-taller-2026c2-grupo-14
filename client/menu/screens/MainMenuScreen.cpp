#include "client/menu/screens/MainMenuScreen.h"

#include <QLabel>
#include <QVBoxLayout>

#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"

MainMenuScreen::MainMenuScreen(QWidget* parent): BaseScreen(parent) {
    auto* newGameButton = new MenuButton("Nueva partida", this);
    auto* joinGameButton = new MenuButton("Unirse a partida", this);
    auto* myGamesButton = new MenuButton("Mis partidas", this);
    auto* exitButton = new MenuButton("Salir", this);

    // Columna angosta para que los botones no ocupen todo el ancho de la ventana.
    auto* buttonsColumn = new QWidget(this);
    buttonsColumn->setFixedWidth(Theme::MENU_COLUMN_WIDTH);
    auto* buttonsLayout = new QVBoxLayout(buttonsColumn);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(Theme::SCREEN_SPACING);
    buttonsLayout->addWidget(newGameButton);
    buttonsLayout->addWidget(joinGameButton);
    buttonsLayout->addWidget(myGamesButton);
    buttonsLayout->addWidget(exitButton);

    mainLayout->addStretch();
    mainLayout->addWidget(createTitle("Stardew Valley"));
    mainLayout->addSpacing(30);
    mainLayout->addWidget(buttonsColumn, 0, Qt::AlignHCenter);
    mainLayout->addStretch();

    connect(newGameButton, &QPushButton::clicked, this, &MainMenuScreen::newGameRequested);
    connect(joinGameButton, &QPushButton::clicked, this, &MainMenuScreen::joinGameRequested);
    connect(myGamesButton, &QPushButton::clicked, this, &MainMenuScreen::myGamesRequested);
    connect(exitButton, &QPushButton::clicked, this, &MainMenuScreen::exitRequested);
}
