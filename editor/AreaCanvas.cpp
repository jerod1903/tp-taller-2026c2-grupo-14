#include "editor/AreaCanvas.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QPen>
#include <QScrollBar>
#include <QString>
#include <QWheelEvent>
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
}  // namespace

AreaCanvas::AreaCanvas(QWidget* parent): QGraphicsView(parent) {
    setScene(&scene);
    scale(INITIAL_ZOOM, INITIAL_ZOOM);
}

void AreaCanvas::showArea(maps::MapDefinition& map, maps::MapArea& shown) {
    scene.clear();
    tileItems.clear();
    area = &shown;
    for (int y = 0; y < static_cast<int>(area->tiles.size()); ++y) {
        std::vector<QGraphicsPixmapItem*>& row = tileItems.emplace_back();
        for (int x = 0; x < static_cast<int>(area->tiles[y].size()); ++x) {
            QGraphicsPixmapItem* item = scene.addPixmap(sprites.tile(area->tiles[y][x]));
            item->setPos(x * TILE, y * TILE);
            row.push_back(item);
        }
    }
    for (const maps::MapElement& element: map.elements) {
        if (element.areaId == area->id) {
            addElement(element);
        }
    }
    scene.setSceneRect(0, 0, area->width * TILE, area->height * TILE);
}

void AreaCanvas::setBrush(maps::TileType tile) { brush = tile; }

void AreaCanvas::addElement(const maps::MapElement& element) {
    const int width = element.width * TILE;
    const int bottom = (element.y + element.height) * TILE;
    // Un mapa con errores puede traer elementos sin tipo: se abren igual para poder corregirlos
    std::optional<QPixmap> sprite = element.type ? sprites.element(*element.type) : std::nullopt;
    QGraphicsItem* item = nullptr;
    if (sprite) {
        // Como en el juego: al ancho del elemento y apoyado en su base, el techo sobresale arriba
        QPixmap scaled = sprite->scaledToWidth(width);
        item = scene.addPixmap(scaled);
        item->setPos(element.x * TILE, bottom - scaled.height());
    } else {
        item = scene.addRect(element.x * TILE, element.y * TILE, width, element.height * TILE,
                             QPen(Qt::NoPen), QBrush(UNDRAWN_ELEMENT));
    }
    item->setZValue(ELEMENTS_Z + bottom);
    item->setToolTip(QString::fromStdString(element.id));
}

void AreaCanvas::wheelEvent(QWheelEvent* event) {
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

void AreaCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        paintAt(event->pos());
    }
}

void AreaCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        paintAt(event->pos());
    }
}

void AreaCanvas::paintAt(const QPoint& viewPosition) {
    if (!area) {
        return;
    }
    const QPointF position = mapToScene(viewPosition);
    const int x = static_cast<int>(position.x()) / TILE;
    const int y = static_cast<int>(position.y()) / TILE;
    if (position.x() < 0 || position.y() < 0 || y >= static_cast<int>(tileItems.size()) ||
        x >= static_cast<int>(tileItems[y].size()) || area->tiles[y][x] == brush) {
        return;
    }
    area->tiles[y][x] = brush;
    tileItems[y][x]->setPixmap(sprites.tile(brush));
}
