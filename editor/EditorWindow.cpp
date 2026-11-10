#include "editor/EditorWindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QScreen>
#include <QStackedWidget>
#include <QString>
#include <QVBoxLayout>
#include <algorithm>
#include <exception>

#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"
#include "common/map/persistence/MapRequirementsLoader.h"
#include "editor/MapEditorScreen.h"
#include "editor/Messages.h"
#include "editor/NameDialog.h"

namespace {
// Mas grande que la ventana del juego: entran el mapa y el panel de elementos. Se puede agrandar a
// mano, pero no achicar por debajo del minimo, donde todavia entra todo.
constexpr int WINDOW_WIDTH = 1680;
constexpr int WINDOW_HEIGHT = 1000;
constexpr int MIN_WINDOW_WIDTH = 1440;
constexpr int MIN_WINDOW_HEIGHT = 900;
// Solo el fondo, del marron del menu
const char* const WINDOW_STYLE = "QMainWindow { background-color: #2D1B10; }";

// El archivo de un mapa sale de su nombre: "Valle Ñandú" es valle-nandu. Minusculas, guiones en
// vez de espacios y sin tildes (las letras con tilde se separan en letra mas tilde y la tilde se
// descarta). Lo que no es letra ni numero se saltea.
QString fileNameFor(const QString& name) {
    QString file;
    for (const QChar c: name.normalized(QString::NormalizationForm_D).toLower()) {
        if (c.isLetterOrNumber() && c.unicode() < 128) {
            file += c;
        } else if ((c.isSpace() || c == '-' || c == '_') && !file.isEmpty() &&
                   !file.endsWith('-')) {
            file += '-';
        }
    }
    while (file.endsWith('-')) {
        file.chop(1);
    }
    return file;
}
}  // namespace

EditorWindow::EditorWindow(QWidget* parent):
        QMainWindow(parent),
        service(repository, stardew::maps::loadMapRequirements(TALLER_MAPS_DIR
                                                               "/config/map_requirements.toml")),
        stack(new QStackedWidget(this)),
        menu(createMenu()),
        editor(new MapEditorScreen(service, stack)) {
    setWindowTitle("Stardew Valley - Editor de mapas");
    setMinimumSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
    // Arranca con el tamano de siempre, o con lo que entre en una pantalla mas chica
    const QSize available = QGuiApplication::primaryScreen()->availableSize();
    resize(std::min(WINDOW_WIDTH, available.width()), std::min(WINDOW_HEIGHT, available.height()));
    // El estilo del juego va en el stack, no en la ventana: asi lo toman el menu, el editor y sus
    // cuadros, y el selector de archivos (hijo de la ventana) queda con el estilo de siempre
    setStyleSheet(WINDOW_STYLE);
    stack->setStyleSheet(StyleManager::globalStyleSheet());
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
    auto* newMapButton = new MenuButton("Nuevo mapa", screen);
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

    connect(newMapButton, &QPushButton::clicked, this, &EditorWindow::newMap);
    connect(openMapButton, &QPushButton::clicked, this, &EditorWindow::openMap);
    connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
    return screen;
}

void EditorWindow::newMap() {
    NameDialog dialog(stack);
    while (dialog.exec() == QDialog::Accepted) {
        const QString file = fileNameFor(dialog.name());
        const QString path = QString(TALLER_MAPS_DIR) + "/" + file + ".toml";
        if (file.isEmpty()) {
            tell(stack, QMessageBox::Warning, "Nuevo mapa",
                 "El nombre debe contener al menos una letra o un número.");
        } else if (QFileInfo::exists(path)) {
            tell(stack, QMessageBox::Warning, "Nuevo mapa",
                 "Ya existe un mapa con ese nombre (" + file + ".toml). Por favor, elija otro.");
        } else {
            try {
                editor->create(dialog.name().toStdString(), path.toStdString());
                stack->setCurrentWidget(editor);
            } catch (const std::exception& e) {
                tell(stack, QMessageBox::Critical, "Nuevo mapa", e.what());
            }
            return;
        }
    }
}

void EditorWindow::openMap() {
    // Hijo de la ventana: se abre encima de ella, como popup, y al cerrarse la ventana vuelve a
    // tener el foco
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
        tell(stack, QMessageBox::Critical, "Abrir mapa", e.what());
    }
}
