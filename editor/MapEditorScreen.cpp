#include "editor/MapEditorScreen.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QIcon>
#include <QMessageBox>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <exception>
#include <utility>

#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"
#include "editor/MapCanvas.h"

namespace maps = stardew::maps;

namespace {
constexpr int PALETTE_ICON_SIZE = 32;
constexpr int PALETTE_BUTTON_PADDING = 8;

constexpr int AREA_GAP = 2;
constexpr int MAX_ROW_WIDTH = 100;

bool areasOverlap(const std::vector<maps::MapValidationIssue>& issues) {
    return std::any_of(issues.begin(), issues.end(), [](const auto& issue) {
        return issue.code == maps::MapIssueCode::AreasOverlap;
    });
}

// Pone las areas en filas, en el orden del archivo, con un espacio entre cada una
void arrangeAreas(maps::MapDefinition& map) {
    int x = 0;
    int y = 0;
    int rowHeight = 0;
    for (maps::MapArea& area: map.areas) {
        if (x > 0 && x + area.width > MAX_ROW_WIDTH) {
            x = 0;
            y += rowHeight + AREA_GAP;
            rowHeight = 0;
        }
        area.offsetX = x;
        area.offsetY = y;
        x += area.width + AREA_GAP;
        rowHeight = std::max(rowHeight, area.height);
    }
}

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
    if (issue.relatedAreaId) {
        text += " y " + QString::fromStdString(*issue.relatedAreaId);
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
        QWidget(parent), service(service), canvas(new MapCanvas(editing, this)) {
    canvas->setTool(&paintTool);
    auto* backButton = new MenuButton("← Volver", this);
    backButton->setBack();
    auto* validateButton = new MenuButton("Validar", this);
    validateButton->setSmall();
    auto* saveButton = new MenuButton("Guardar", this);
    saveButton->setSmall();

    auto* header = new QHBoxLayout();
    header->addWidget(backButton);
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
        connect(button, &QToolButton::clicked, this, [this, tile]() {
            paintTool.setTile(tile);
            canvas->setTool(&paintTool);
        });
    }
    // Un click en el borde entre dos areas pone (o quita) un porton de cada lado
    auto* passageButton = new QToolButton(this);
    passageButton->setText("Pasaje");
    passageButton->setToolTip("Click en el borde entre dos areas: pone o quita un pasaje");
    passageButton->setCheckable(true);
    passageButton->setMinimumHeight(PALETTE_ICON_SIZE + PALETTE_BUTTON_PADDING);
    group->addButton(passageButton);
    palette->addWidget(passageButton);
    connect(passageButton, &QToolButton::clicked, this,
            [this]() { canvas->setTool(&passageTool); });
    palette->addStretch();
    return palette;
}

void MapEditorScreen::open(const std::string& mapPath) {
    maps::MapLoadResult result = service.loadMap(mapPath);
    path = mapPath;
    // Un mapa sin posiciones tiene todas las areas en (0, 0): se acomodan para poder verlas.
    // Quedan asi cuando se guarde.
    if (areasOverlap(result.issues)) {
        arrangeAreas(result.map);
        result.issues = service.validate(result.map);
    }
    editing.load(std::move(result.map));
    if (!result.issues.empty()) {
        showIssues("El mapa tiene errores", result.issues);
    }
}

void MapEditorScreen::validate() {
    std::vector<maps::MapValidationIssue> issues = service.validate(editing.map());
    if (issues.empty()) {
        QMessageBox::information(this, "Validar", "El mapa es valido.");
    } else {
        showIssues("El mapa tiene errores", issues);
    }
}

void MapEditorScreen::save() {
    try {
        std::vector<maps::MapValidationIssue> issues = service.saveMap(path, editing.map());
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
