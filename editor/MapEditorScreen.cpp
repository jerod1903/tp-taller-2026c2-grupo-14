#include "editor/MapEditorScreen.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>
#include <exception>
#include <utility>

#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"
#include "editor/AreaCanvas.h"

namespace maps = stardew::maps;

namespace {
constexpr int PALETTE_ICON_SIZE = 32;

QString describe(const maps::MapValidationIssue& issue) {
    QString text = QString::fromStdString(std::string(maps::toString(issue.code)));
    if (issue.areaType) {
        text += " · " + QString::fromStdString(std::string(maps::toString(*issue.areaType)));
    }
    if (issue.floorNumber) {
        text += QString(" · piso %1").arg(*issue.floorNumber);
    }
    if (issue.elementType) {
        text += " · " + QString::fromStdString(std::string(maps::toString(*issue.elementType)));
    }
    if (issue.expected && issue.actual) {
        text += QString(" · se esperaban %1, hay %2").arg(*issue.expected).arg(*issue.actual);
    }
    if (issue.areaId) {
        text += " · area " + QString::fromStdString(*issue.areaId);
    }
    if (issue.elementId) {
        text += " · elemento " + QString::fromStdString(*issue.elementId);
    }
    if (issue.relatedElementId) {
        text += " y " + QString::fromStdString(*issue.relatedElementId);
    }
    if (issue.x && issue.y) {
        text += QString(" · (%1, %2)").arg(*issue.x).arg(*issue.y);
    }
    return text;
}
}  // namespace

MapEditorScreen::MapEditorScreen(const maps::MapService& service, QWidget* parent):
        QWidget(parent),
        service(service),
        areaSelector(new QComboBox(this)),
        canvas(new AreaCanvas(this)) {
    auto* backButton = new MenuButton("← Volver", this);
    backButton->setBack();
    auto* validateButton = new MenuButton("Validar", this);
    validateButton->setSmall();
    auto* saveButton = new MenuButton("Guardar", this);
    saveButton->setSmall();

    auto* header = new QHBoxLayout();
    header->addWidget(backButton);
    header->addWidget(new QLabel("Area:", this));
    header->addWidget(areaSelector);
    header->addStretch();
    header->addWidget(validateButton);
    header->addWidget(saveButton);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SCREEN_MARGIN_V, Theme::SCREEN_MARGIN_V,
                               Theme::SCREEN_MARGIN_V, Theme::SCREEN_MARGIN_V);
    layout->addLayout(header);
    layout->addLayout(createPalette());
    layout->addWidget(canvas, 1);

    connect(backButton, &QPushButton::clicked, this, &MapEditorScreen::backRequested);
    connect(validateButton, &QPushButton::clicked, this, &MapEditorScreen::validate);
    connect(saveButton, &QPushButton::clicked, this, &MapEditorScreen::save);
    connect(areaSelector, &QComboBox::currentIndexChanged, this,
            &MapEditorScreen::showSelectedArea);
}

QHBoxLayout* MapEditorScreen::createPalette() {
    auto* palette = new QHBoxLayout();
    auto* group = new QButtonGroup(this);
    for (maps::TileType tile: maps::allTileTypes()) {
        auto* button = new QToolButton(this);
        button->setIcon(QIcon(sprites.tile(tile)));
        button->setIconSize(QSize(PALETTE_ICON_SIZE, PALETTE_ICON_SIZE));
        button->setToolTip(QString::fromStdString(std::string(maps::toString(tile))));
        button->setCheckable(true);
        button->setChecked(tile == maps::TileType::Grass);
        group->addButton(button);
        palette->addWidget(button);
        connect(button, &QToolButton::clicked, this, [this, tile]() { canvas->setBrush(tile); });
    }
    palette->addStretch();
    return palette;
}

void MapEditorScreen::open(const std::string& mapPath) {
    maps::MapLoadResult result = service.loadMap(mapPath);
    map = std::move(result.map);
    path = mapPath;
    // Sin senales mientras se llena: showSelectedArea se llama una vez al final
    areaSelector->blockSignals(true);
    areaSelector->clear();
    for (const maps::MapArea& area: map.areas) {
        areaSelector->addItem(QString::fromStdString(area.id));
    }
    areaSelector->blockSignals(false);
    showSelectedArea();
    if (!result.issues.empty()) {
        showIssues("El mapa tiene errores", result.issues);
    }
}

void MapEditorScreen::showSelectedArea() {
    const int index = areaSelector->currentIndex();
    if (index >= 0) {
        canvas->showArea(map, map.areas[index]);
    }
}

void MapEditorScreen::validate() {
    std::vector<maps::MapValidationIssue> issues = service.validate(map);
    if (issues.empty()) {
        QMessageBox::information(this, "Validar", "El mapa es valido.");
    } else {
        showIssues("El mapa tiene errores", issues);
    }
}

void MapEditorScreen::save() {
    try {
        std::vector<maps::MapValidationIssue> issues = service.saveMap(path, map);
        if (issues.empty()) {
            QMessageBox::information(this, "Guardar", "Mapa guardado.");
        } else {
            showIssues("No se guardo: el mapa tiene errores", issues);
        }
    } catch (const std::exception& e) {
        // La excepcion no puede salir del slot: atravesaria el codigo de Qt
        QMessageBox::critical(this, "Guardar", e.what());
    }
}

void MapEditorScreen::showIssues(const QString& title,
                                 const std::vector<maps::MapValidationIssue>& issues) {
    QStringList lines;
    for (const maps::MapValidationIssue& issue: issues) {
        lines << describe(issue);
    }
    QMessageBox::warning(this, title, lines.join("\n"));
}
