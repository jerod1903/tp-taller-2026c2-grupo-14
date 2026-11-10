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
#include <optional>
#include <string>
#include <utility>

#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"
#include "editor/BlockPalette.h"
#include "editor/MapCanvas.h"
#include "editor/MapHelper.h"
#include "editor/Messages.h"
#include "editor/MineHelper.h"
#include "editor/NewMap.h"
#include "editor/SizeDialog.h"

namespace maps = stardew::maps;

namespace {
constexpr int PALETTE_ICON_SIZE = 32;
constexpr int PALETTE_BUTTON_PADDING = 8;
// Los tiles, en una columna con su nombre al lado
constexpr int TILE_ICON_SIZE = 40;
constexpr int TILE_BUTTON_WIDTH = 160;

// Los pisos de la mina: lo justo para la pared, la salida, la escalera y algo de piso, y mas de 30
// es tedioso de editar. Por defecto, los de valle.toml.
const SizeRequest FLOOR_SIZE{"Pisos de la mina",
                             "Todos los pisos miden lo mismo y después no se puede cambiar.",
                             5,
                             30,
                             {20, 15}};
// La granja y el pueblo: por defecto, los de valle.toml. El titulo lo pone quien lo pide.
const SizeRequest AREA_SIZE{"",
                            "Para cambiar el tamaño después, hay que borrarla y crearla de nuevo.",
                            10,
                            60,
                            {30, 20}};

// Barras finas del color de los botones de madera, sin las flechas de los extremos. Se ponen en la
// pantalla para que valgan en el lienzo y en el panel de elementos.
const char* const SCROLLBARS_STYLE = R"(
    QScrollBar:horizontal {
        height: 12px;
        background: #2D1B10;
    }
    QScrollBar:vertical {
        width: 12px;
        background: #2D1B10;
    }
    QScrollBar::handle {
        background: #8E4E46;
        border-radius: 5px;
        min-width: 40px;
        min-height: 40px;
    }
    QScrollBar::handle:hover {
        background: #A65D52;
    }
    QScrollBar::add-line, QScrollBar::sub-line {
        width: 0;
        height: 0;
    }
    QScrollBar::add-page, QScrollBar::sub-page {
        background: none;
    }
    QAbstractScrollArea::corner {
        background: #2D1B10;
    }
)";

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

// Un error del validador en una linea: el codigo y, con su nombre, cada dato que trae. Por ejemplo
// "AreaCountMismatch: tipo de area farm, esperado 1, encontrado 2".
QString describe(const maps::MapValidationIssue& issue) {
    QStringList data;
    if (issue.areaType) {
        data << "tipo de área " +
                        QString::fromStdString(std::string(maps::toString(*issue.areaType)));
    }
    if (issue.floorNumber) {
        data << QString("piso %1").arg(*issue.floorNumber);
    }
    if (issue.elementType) {
        data << "tipo de elemento " +
                        QString::fromStdString(std::string(maps::toString(*issue.elementType)));
    }
    if (issue.expected && issue.actual) {
        data << QString("esperado %1, encontrado %2").arg(*issue.expected).arg(*issue.actual);
    }
    if (issue.areaId) {
        QString areas = "área " + QString::fromStdString(*issue.areaId);
        if (issue.relatedAreaId) {
            areas += " y " + QString::fromStdString(*issue.relatedAreaId);
        }
        data << areas;
    }
    if (issue.elementId) {
        QString elements = "elemento " + QString::fromStdString(*issue.elementId);
        if (issue.relatedElementId) {
            elements += " y " + QString::fromStdString(*issue.relatedElementId);
        }
        data << elements;
    }
    if (issue.x && issue.y) {
        data << QString("casilla (%1, %2)").arg(*issue.x).arg(*issue.y);
    }
    const QString code = QString::fromStdString(std::string(maps::toString(issue.code)));
    return data.isEmpty() ? code : code + ": " + data.join(", ");
}
}  // namespace

MapEditorScreen::MapEditorScreen(const maps::MapService& service, QWidget* parent):
        QWidget(parent), service(service), canvas(new MapCanvas(editing, this)) {
    setStyleSheet(SCROLLBARS_STYLE);
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

    // Una sola herramienta marcada entre los botones de arriba y las tarjetas del panel
    auto* group = new QButtonGroup(this);
    auto* blocks = new BlockPalette(service.requirements(), sprites, group, this);
    // Los tiles a la izquierda del mapa y lo que se pone, a la derecha
    auto* body = new QHBoxLayout();
    body->addLayout(createTiles(group));
    body->addWidget(canvas, 1);
    body->addWidget(blocks);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::SCREEN_MARGIN_V, Theme::SCREEN_MARGIN_V,
                               Theme::SCREEN_MARGIN_V, Theme::SCREEN_MARGIN_V);
    layout->addLayout(header);
    layout->addLayout(createTools(group));
    layout->addLayout(body, 1);

    // La pantalla une la seleccion (de un elemento o de un area) con el lienzo y con el boton
    // Borrar. Al cambiar de herramienta se limpia, asi que hay una sola a la vez.
    selectTool.onSelectionChanged([this](const std::optional<std::string>& elementId) {
        canvas->showSelection(elementId);
        removeButton->setEnabled(elementId.has_value());
    });
    areaTool.onSelectionChanged([this](const std::optional<std::string>& areaId) {
        canvas->showAreaSelection(areaId);
        removeButton->setEnabled(areaId.has_value());
    });
    // Los fantasmas de lo que se arrastra (un elemento o un area) van al lienzo
    selectTool.onGhostChanged(
            [this](const std::optional<Ghost>& ghost) { canvas->showGhost(ghost); });
    areaTool.onGhostChanged(
            [this](const std::optional<AreaGhost>& ghost) { canvas->showAreaGhost(ghost); });
    areaTool.onConfirmBrokenPassages([this](int brokenPassages) {
        const QString text =
                brokenPassages == 1 ?
                        QString("Al mover el área, un pasaje queda separado y será borrado. "
                                "¿Mover igual?") :
                        QString("Al mover el área, %1 pasajes quedan separados y serán borrados. "
                                "¿Mover igual?")
                                .arg(brokenPassages);
        return ask(this, "Mover área", text);
    });
    connectPlacing(placeTool);
    connectPlacing(entranceTool);
    placeAreaTool.onGhostChanged(
            [this](const std::optional<AreaGhost>& ghost) { canvas->showAreaGhost(ghost); });
    placeAreaTool.onDone([this]() { backToSelect(); });

    connect(blocks, &BlockPalette::areaChosen, this, &MapEditorScreen::chooseArea);

    connect(blocks, &BlockPalette::typeChosen, this, [this](maps::ElementType type) {
        if (type == maps::ElementType::DungeonEntrance) {
            chooseEntrance();
            return;
        }
        placeTool.setType(type);
        useTool(&placeTool);
    });
    connect(backButton, &QPushButton::clicked, this, &MapEditorScreen::goBack);
    connect(validateButton, &QPushButton::clicked, this, &MapEditorScreen::validate);
    connect(saveButton, &QPushButton::clicked, this, [this]() { save(true); });
}

QVBoxLayout* MapEditorScreen::createTiles(QButtonGroup* group) {
    auto* tiles = new QVBoxLayout();
    for (maps::TileType tile: maps::allTileTypes()) {
        auto* button = new QToolButton(this);
        // El tile mide una casilla: se agranda al tamano del icono (el icono no agranda solo)
        button->setIcon(QIcon(sprites.tile(tile).scaled(TILE_ICON_SIZE, TILE_ICON_SIZE)));
        button->setIconSize(QSize(TILE_ICON_SIZE, TILE_ICON_SIZE));
        button->setText(QString::fromStdString(std::string(maps::toString(tile))));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setFixedWidth(TILE_BUTTON_WIDTH);
        button->setCheckable(true);
        button->setChecked(tile == maps::TileType::Grass);
        group->addButton(button);
        tiles->addWidget(button);
        connect(button, &QToolButton::clicked, this, [this, tile]() {
            paintTool.setTile(tile);
            useTool(&paintTool);
        });
    }
    tiles->addStretch();
    return tiles;
}

QHBoxLayout* MapEditorScreen::createTools(QButtonGroup* group) {
    auto* palette = new QHBoxLayout();
    // Centradas: un estiramiento de cada lado
    palette->addStretch();
    // Un click en el borde entre dos areas pone (o quita) un porton de cada lado
    auto* passageButton = new QToolButton(this);
    passageButton->setText("Editar pasajes");
    passageButton->setToolTip("Click en el borde entre dos áreas: pone o quita un pasaje");
    passageButton->setCheckable(true);
    passageButton->setMinimumHeight(PALETTE_ICON_SIZE + PALETTE_BUTTON_PADDING);
    group->addButton(passageButton);
    palette->addWidget(passageButton);
    connect(passageButton, &QToolButton::clicked, this, [this]() { useTool(&passageTool); });

    auto* areaButton = new QToolButton(this);
    areaButton->setText("Editar áreas");
    areaButton->setToolTip("Click en un área para seleccionarla (Borrar la quita); arrastrarla la "
                           "mueve con todo. Los pisos de la mina se mueven juntos.");
    areaButton->setCheckable(true);
    areaButton->setMinimumHeight(PALETTE_ICON_SIZE + PALETTE_BUTTON_PADDING);
    group->addButton(areaButton);
    palette->addWidget(areaButton);
    connect(areaButton, &QToolButton::clicked, this, [this]() { useTool(&areaTool); });

    selectButton = new QToolButton(this);
    selectButton->setText("Seleccionar elemento");
    selectButton->setToolTip(
            "Click en la base de un elemento para seleccionarlo; arrastrarlo lo mueve");
    selectButton->setCheckable(true);
    selectButton->setMinimumHeight(PALETTE_ICON_SIZE + PALETTE_BUTTON_PADDING);
    group->addButton(selectButton);
    palette->addWidget(selectButton);
    connect(selectButton, &QToolButton::clicked, this, [this]() { useTool(&selectTool); });

    // No es una herramienta: borra lo seleccionado. Sin seleccion no se puede apretar.
    removeButton = new QToolButton(this);
    removeButton->setText("Borrar");
    removeButton->setToolTip("Borra el elemento o el área seleccionada");
    removeButton->setEnabled(false);
    removeButton->setMinimumHeight(PALETTE_ICON_SIZE + PALETTE_BUTTON_PADDING);
    palette->addWidget(removeButton);
    connect(removeButton, &QToolButton::clicked, this, &MapEditorScreen::removeSelected);
    palette->addStretch();
    return palette;
}

void MapEditorScreen::connectPlacing(PlaceTool& tool) {
    tool.onGhostChanged([this](const std::optional<Ghost>& ghost) { canvas->showGhost(ghost); });
    tool.onDone([this]() { backToSelect(); });
}

void MapEditorScreen::chooseEntrance() {
    std::optional<AreaSize> floorSize;
    if (!hasDungeonFloors(editing.map())) {
        SizeDialog dialog(FLOOR_SIZE, this);
        if (dialog.exec() != QDialog::Accepted) {
            backToSelect();
            return;
        }
        floorSize = dialog.size();
    }
    entranceTool.setFloorSize(floorSize);
    useTool(&entranceTool);
}

void MapEditorScreen::chooseArea(maps::AreaType type) {
    SizeRequest request = AREA_SIZE;
    request.title = type == maps::AreaType::Farm ? "Granja nueva" : "Pueblo nuevo";
    SizeDialog dialog(request, this);
    if (dialog.exec() != QDialog::Accepted) {
        backToSelect();
        return;
    }
    placeAreaTool.setArea(type, dialog.size());
    useTool(&placeAreaTool);
}

void MapEditorScreen::backToSelect() {
    selectButton->setChecked(true);
    useTool(&selectTool);
}

void MapEditorScreen::removeSelected() {
    if (activeTool != &areaTool) {
        selectTool.removeSelected();
        return;
    }
    const std::optional<AreaRemoval> removal =
            areaTool.selected() ? editing.areaRemovalOf(*areaTool.selected()) : std::nullopt;
    if (!removal) {
        return;
    }
    const bool floors = removal->areaIds.size() > 1;
    const QString areas = floors ? QString("los %1 pisos de la mina").arg(removal->areaIds.size()) :
                                   QString::fromStdString(removal->areaIds.front());
    QString elements;
    if (removal->elements == 1) {
        elements = " y su elemento";
    } else if (removal->elements > 1) {
        elements = QString(" y sus %1 elementos").arg(removal->elements);
    }
    const QString text =
            "¿Borrar " + areas + elements + "? También se borran sus pasajes y las " +
            (floors ? "transiciones que llegan a ellos." : "transiciones que llegan a ella.");
    if (ask(this, "Borrar área", text)) {
        areaTool.removeSelected();
    }
}

void MapEditorScreen::useTool(Tool* tool) {
    selectTool.clear();
    areaTool.clear();
    canvas->showGhost(std::nullopt);
    canvas->showAreaGhost(std::nullopt);
    canvas->setTool(tool);
    activeTool = tool;
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
    show(std::move(result.map));
    if (!result.issues.empty()) {
        showIssues("El mapa tiene errores", result.issues);
    }
}

void MapEditorScreen::create(const std::string& name, const std::string& mapPath) {
    path = mapPath;
    show(newMap(name, service.requirements().formatVersion));
    // Se guarda ya, para que el archivo exista (todavia con errores: le falta la mina)
    save(false);
}

void MapEditorScreen::show(maps::MapDefinition map) {
    selectTool.clear();
    areaTool.clear();
    editing.load(std::move(map));
}

void MapEditorScreen::goBack() {
    if (editing.hasUnsavedChanges() &&
        !ask(this, "Volver", "Hay cambios sin guardar. ¿Salir igual?")) {
        return;
    }
    emit backRequested();
}

void MapEditorScreen::validate() {
    std::vector<maps::MapValidationIssue> issues = service.validate(editing.map());
    if (issues.empty()) {
        tell(this, QMessageBox::Information, "Validar", "El mapa es válido.");
    } else {
        showIssues("El mapa tiene errores", issues);
    }
}

void MapEditorScreen::save(bool report) {
    try {
        std::vector<maps::MapValidationIssue> issues = service.saveMap(path, editing.map());
        editing.markSaved();
        if (!report) {
            return;
        }
        if (issues.empty()) {
            tell(this, QMessageBox::Information, "Guardar", "Mapa guardado.");
        } else {
            showIssues("El mapa se guardó, pero tiene errores: deben corregirse antes de jugarlo",
                       issues);
        }
    } catch (const std::exception& e) {
        // La excepcion no puede salir del slot: atravesaria el codigo de Qt
        tell(this, QMessageBox::Critical, "Guardar", e.what());
    }
}

void MapEditorScreen::showIssues(const QString& title,
                                 const std::vector<maps::MapValidationIssue>& issues) {
    QStringList lines;
    for (const maps::MapValidationIssue& issue: issues) {
        lines << describe(issue);
    }
    tell(this, QMessageBox::Warning, title, lines.join("\n"), true);
}
