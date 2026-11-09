#include "editor/EditorWindow.h"

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <exception>

#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"
#include "common/map/persistence/MapRequirementsLoader.h"
#include "editor/MapEditorScreen.h"

namespace {
// Mas grande que la ventana del juego: entran el mapa y el panel de elementos
constexpr int WINDOW_WIDTH = 1440;
constexpr int WINDOW_HEIGHT = 900;
}  // namespace

EditorWindow::EditorWindow(QWidget* parent):
        QMainWindow(parent),
        service(repository, stardew::maps::loadMapRequirements(TALLER_MAPS_DIR
                                                               "/config/map_requirements.toml")),
        stack(new QStackedWidget(this)),
        menu(createMenu()),
        editor(new MapEditorScreen(service, stack)) {
    setWindowTitle("Stardew Valley - Editor de mapas");
    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setStyleSheet(StyleManager::globalStyleSheet());
    stack->addWidget(menu);
    stack->addWidget(editor);
    setCentralWidget(stack);

    connect(editor, &MapEditorScreen::backRequested, this,
            [this]() { stack->setCurrentWidget(menu); });
}

QWidget* EditorWindow::createMenu() {
    auto* screen = new QWidget(stack);
    auto* title = new QLabel("Editor de mapas", screen);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    // Crear se conecta cuando existan las plantillas de mundo
    auto* newMapButton = new MenuButton("Nuevo mapa", screen);
    newMapButton->setEnabled(false);
    auto* openMapButton = new MenuButton("Abrir mapa", screen);
    auto* exitButton = new MenuButton("Salir", screen);

    auto* layout = new QVBoxLayout(screen);
    layout->setSpacing(Theme::SCREEN_SPACING);
    layout->addStretch();
    layout->addWidget(title);
    layout->addSpacing(30);
    layout->addWidget(newMapButton, 0, Qt::AlignHCenter);
    layout->addWidget(openMapButton, 0, Qt::AlignHCenter);
    layout->addWidget(exitButton, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(openMapButton, &QPushButton::clicked, this, &EditorWindow::openMap);
    connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
    return screen;
}

void EditorWindow::openMap() {
    const QString path =
            QFileDialog::getOpenFileName(this, "Abrir mapa", TALLER_MAPS_DIR, "Mapas (*.toml)");
    if (path.isEmpty()) {
        return;
    }
    try {
        editor->open(path.toStdString());
        stack->setCurrentWidget(editor);
    } catch (const std::exception& e) {
        // Un TOML mal escrito no se puede abrir; la excepcion no puede salir del slot
        QMessageBox::critical(this, "Abrir mapa", e.what());
    }
}
