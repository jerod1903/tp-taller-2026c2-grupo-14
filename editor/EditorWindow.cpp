#include "editor/EditorWindow.h"

#include <QLabel>
#include <QVBoxLayout>

#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"

EditorWindow::EditorWindow(QWidget* parent): QMainWindow(parent) {
    setWindowTitle("Stardew Valley - Editor de mapas");
    setFixedSize(Theme::WINDOW_WIDTH, Theme::WINDOW_HEIGHT);
    setStyleSheet(StyleManager::globalStyleSheet());

    auto* menu = new QWidget(this);
    auto* title = new QLabel("Editor de mapas", menu);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    // Crear y abrir se conectan cuando exista el lienzo
    auto* newMapButton = new MenuButton("Nuevo mapa", menu);
    newMapButton->setEnabled(false);
    auto* openMapButton = new MenuButton("Abrir mapa", menu);
    openMapButton->setEnabled(false);
    auto* exitButton = new MenuButton("Salir", menu);

    auto* layout = new QVBoxLayout(menu);
    layout->setSpacing(Theme::SCREEN_SPACING);
    layout->addStretch();
    layout->addWidget(title);
    layout->addSpacing(30);
    layout->addWidget(newMapButton, 0, Qt::AlignHCenter);
    layout->addWidget(openMapButton, 0, Qt::AlignHCenter);
    layout->addWidget(exitButton, 0, Qt::AlignHCenter);
    layout->addStretch();
    setCentralWidget(menu);

    connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
}
