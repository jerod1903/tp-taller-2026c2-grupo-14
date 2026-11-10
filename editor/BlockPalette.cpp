#include "editor/BlockPalette.h"

#include <QButtonGroup>
#include <QColor>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "editor/SpriteSheets.h"

namespace maps = stardew::maps;

namespace {
// Todas las tarjetas del mismo tamano: el dibujo y el nombre abajo, cortado si no entra
constexpr int CARD_SIZE = 96;
constexpr int CARD_ICON_SIZE = 64;
constexpr int CARD_TEXT_PADDING = 8;
constexpr int CARD_SPACING = 6;
// Entran tres tarjetas por fila; las filas con mas se scrollean
constexpr int CARDS_PER_ROW = 3;
constexpr int PANEL_WIDTH = CARDS_PER_ROW * CARD_SIZE + (CARDS_PER_ROW - 1) * CARD_SPACING;
// El mismo recuadro con el que el lienzo dibuja los elementos sin dibujo
const QColor UNDRAWN_ELEMENT(255, 235, 173, 120);

// Las areas que se crean desde el panel, con un terreno que las representa
struct NewArea {
    maps::AreaType type;
    maps::TileType icon;
};

const NewArea NEW_AREAS[] = {
        {maps::AreaType::Farm, maps::TileType::TilledSoil},
        {maps::AreaType::Village, maps::TileType::Path},
};

struct Section {
    maps::AreaType areaType;
    const char* title;
};

const Section SECTIONS[] = {
        {maps::AreaType::Farm, "Granja"},
        {maps::AreaType::Village, "Pueblo"},
        {maps::AreaType::DungeonFloor, "Mina"},
};

// Los que pone el editor solo: los portones con la herramienta Pasaje y la salida y la escalera al
// crear los pisos de la mina
bool placedByEditor(maps::ElementType type) {
    return type == maps::ElementType::Door || type == maps::ElementType::Exit ||
           type == maps::ElementType::Stairs;
}

// Los tipos que se pueden colocar en ese tipo de area, en el orden de las reglas y sin repetir
std::vector<maps::ElementType> placeableIn(const maps::MapRequirements& requirements,
                                           maps::AreaType areaType) {
    std::vector<maps::ElementType> types;
    for (const maps::ElementRule& rule: requirements.elements) {
        if (rule.areaType == areaType && !placedByEditor(rule.type) &&
            std::find(types.begin(), types.end(), rule.type) == types.end()) {
            types.push_back(rule.type);
        }
    }
    return types;
}
}  // namespace

BlockPalette::BlockPalette(const maps::MapRequirements& requirements, SpriteSheets& sprites,
                           QButtonGroup* group, QWidget* parent):
        QWidget(parent) {
    setFixedWidth(PANEL_WIDTH);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel("Areas", this));
    QHBoxLayout* areaCards = addRow(layout);
    for (const NewArea& area: NEW_AREAS) {
        const QString name = QString::fromStdString(std::string(maps::toString(area.type)));
        // El tile mide una casilla: se agranda al tamano del icono (el icono no agranda solo)
        const QPixmap icon = sprites.tile(area.icon).scaled(CARD_ICON_SIZE, CARD_ICON_SIZE);
        QToolButton* card = addCard(areaCards, icon, name, group);
        connect(card, &QToolButton::clicked, this,
                [this, type = area.type]() { emit areaChosen(type); });
    }

    for (const Section& section: SECTIONS) {
        const std::vector<maps::ElementType> types = placeableIn(requirements, section.areaType);
        if (types.empty()) {
            continue;
        }
        layout->addWidget(new QLabel(section.title, this));
        QHBoxLayout* cards = addRow(layout);
        for (maps::ElementType type: types) {
            std::optional<QPixmap> sprite = sprites.element(type);
            if (!sprite) {
                // El spawn no tiene dibujo: el mismo recuadro que en el lienzo
                sprite = QPixmap(CARD_ICON_SIZE, CARD_ICON_SIZE);
                sprite->fill(UNDRAWN_ELEMENT);
            }
            const QString name = QString::fromStdString(std::string(maps::toString(type)));
            QToolButton* card = addCard(cards, *sprite, name, group);
            connect(card, &QToolButton::clicked, this, [this, type]() { emit typeChosen(type); });
        }
    }
    layout->addStretch();
}

QHBoxLayout* BlockPalette::addRow(QVBoxLayout* panel) {
    auto* scroll = new QScrollArea(this);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // La fila mide una tarjeta de alto mas la barra, aunque la barra no se vea
    scroll->setFixedHeight(CARD_SIZE + scroll->horizontalScrollBar()->sizeHint().height());
    scroll->setWidgetResizable(true);
    auto* row = new QWidget(scroll);
    auto* cards = new QHBoxLayout(row);
    cards->setContentsMargins(0, 0, 0, 0);
    cards->setSpacing(CARD_SPACING);
    cards->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    scroll->setWidget(row);
    panel->addWidget(scroll);
    return cards;
}

QToolButton* BlockPalette::addCard(QHBoxLayout* cards, const QPixmap& icon, const QString& name,
                                   QButtonGroup* group) {
    auto* card = new QToolButton(this);
    card->setFixedSize(CARD_SIZE, CARD_SIZE);
    card->setIcon(QIcon(icon));
    card->setIconSize(QSize(CARD_ICON_SIZE, CARD_ICON_SIZE));
    card->setText(
            card->fontMetrics().elidedText(name, Qt::ElideRight, CARD_SIZE - CARD_TEXT_PADDING));
    card->setToolTip(name);
    card->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    card->setCheckable(true);
    group->addButton(card);
    cards->addWidget(card);
    return card;
}
