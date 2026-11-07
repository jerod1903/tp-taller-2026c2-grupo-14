#include "editor/MapCanvas.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QMouseEvent>
#include <QPen>
#include <QScrollBar>
#include <QString>
#include <QWheelEvent>
#include <cmath>
#include <optional>

#include "common/map/MapSprites.h"

namespace maps = stardew::maps;

namespace {
constexpr int TILE = maps::TILE_SIZE;
constexpr double INITIAL_ZOOM = 2.0;
constexpr double ZOOM_STEP = 1.25;
// Los elementos sin dibujo (spawn, puertas, rocas) se marcan con un recuadro
const QColor UNDRAWN_ELEMENT(255, 235, 173, 120);
// Encima del piso; entre elementos, el de mas abajo tapa al de mas arriba
constexpr double ELEMENTS_Z = 1;
// El borde de cada area va encima de todo
constexpr double BORDER_Z = 1e9;
constexpr int SCENE_MARGIN = 2 * TILE;
const QColor AREA_NAME(255, 235, 173);
// El mismo marron del fondo del menu
const QColor BACKGROUND(45, 27, 16);
// Barras finas del color de los botones de madera, sin las flechas de los extremos
const char* const SCROLLBARS_STYLE = R"(
    QGraphicsView {
        border: none;
    }
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
const QColor AREA_BORDER(255, 235, 173, 160);
}  // namespace

MapCanvas::MapCanvas(QWidget* parent): QGraphicsView(parent) {
    setScene(&scene);
    setBackgroundBrush(BACKGROUND);
    setStyleSheet(SCROLLBARS_STYLE);
    scale(INITIAL_ZOOM, INITIAL_ZOOM);
}

void MapCanvas::showMap(maps::MapDefinition& map) {
    scene.clear();
    areas.clear();
    for (maps::MapArea& area: map.areas) {
        addArea(area);
        for (const maps::MapElement& element: map.elements) {
            if (element.areaId == area.id) {
                addElement(area, element);
            }
        }
    }
    // Un margen alrededor para poder ver los bordes con zoom
    scene.setSceneRect(scene.itemsBoundingRect().adjusted(-SCENE_MARGIN, -SCENE_MARGIN,
                                                          SCENE_MARGIN, SCENE_MARGIN));
}

void MapCanvas::addArea(maps::MapArea& area) {
    const QPointF origin(area.offsetX * TILE, area.offsetY * TILE);
    DrawnArea& drawn = areas.emplace_back(DrawnArea{&area, {}});
    for (int y = 0; y < static_cast<int>(area.tiles.size()); ++y) {
        std::vector<QGraphicsPixmapItem*>& row = drawn.tiles.emplace_back();
        for (int x = 0; x < static_cast<int>(area.tiles[y].size()); ++x) {
            QGraphicsPixmapItem* item = scene.addPixmap(sprites.tile(area.tiles[y][x]));
            item->setPos(origin + QPointF(x * TILE, y * TILE));
            row.push_back(item);
        }
    }
    // El nombre arriba del area y un borde, para ver donde termina cada una
    QGraphicsSimpleTextItem* name = scene.addSimpleText(QString::fromStdString(area.id));
    name->setBrush(AREA_NAME);
    name->setPos(origin - QPointF(0, name->boundingRect().height()));
    QGraphicsRectItem* border = scene.addRect(origin.x(), origin.y(), area.width * TILE,
                                              area.height * TILE, QPen(AREA_BORDER));
    border->setZValue(BORDER_Z);
}

void MapCanvas::setBrush(maps::TileType tile) { brush = tile; }

void MapCanvas::addElement(const maps::MapArea& area, const maps::MapElement& element) {
    const int left = (area.offsetX + element.x) * TILE;
    const int width = element.width * TILE;
    const int bottom = (area.offsetY + element.y + element.height) * TILE;
    // Un mapa con errores puede traer elementos sin tipo: se abren igual para poder corregirlos
    std::optional<QPixmap> sprite = element.type ? sprites.element(*element.type) : std::nullopt;
    QGraphicsItem* item = nullptr;
    if (sprite) {
        // al ancho del elemento y apoyado en su base, el techo sobresale arriba
        QPixmap scaled = sprite->scaledToWidth(width);
        item = scene.addPixmap(scaled);
        item->setPos(left, bottom - scaled.height());
    } else {
        item = scene.addRect(left, bottom - element.height * TILE, width, element.height * TILE,
                             QPen(Qt::NoPen), QBrush(UNDRAWN_ELEMENT));
    }
    item->setZValue(ELEMENTS_Z + bottom);
    item->setToolTip(QString::fromStdString(element.id));
}

void MapCanvas::wheelEvent(QWheelEvent* event) {
    const double factor = event->angleDelta().y() > 0 ? ZOOM_STEP : 1 / ZOOM_STEP;
    // Lo que estaba bajo el mouse tiene que seguir bajo el mouse: despues de escalar se corre la
    // vista lo que se haya movido ese punto
    const QPoint mouse = event->position().toPoint();
    const QPointF anchor = mapToScene(mouse);
    scale(factor, factor);
    const QPoint drift = mapFromScene(anchor) - mouse;
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + drift.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() + drift.y());
}

void MapCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        paintAt(event->pos());
    }
}

void MapCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        paintAt(event->pos());
    }
}

void MapCanvas::paintAt(const QPoint& viewPosition) {
    const QPointF position = mapToScene(viewPosition);
    for (DrawnArea& drawn: areas) {
        // Casilla relativa al area; floor para que -0.5 no caiga en la columna 0
        const int x = static_cast<int>(std::floor(position.x() / TILE)) - drawn.area->offsetX;
        const int y = static_cast<int>(std::floor(position.y() / TILE)) - drawn.area->offsetY;
        if (x < 0 || y < 0 || y >= static_cast<int>(drawn.tiles.size()) ||
            x >= static_cast<int>(drawn.tiles[y].size())) {
            continue;
        }
        if (drawn.area->tiles[y][x] != brush) {
            drawn.area->tiles[y][x] = brush;
            drawn.tiles[y][x]->setPixmap(sprites.tile(brush));
        }
        return;
    }
}
