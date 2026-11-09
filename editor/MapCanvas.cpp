#include "editor/MapCanvas.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsColorizeEffect>
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
#include "editor/MapEditing.h"
#include "editor/MapHelper.h"
#include "editor/tools/Tool.h"

namespace maps = stardew::maps;

namespace {
constexpr int TILE = maps::TILE_SIZE;
constexpr double INITIAL_ZOOM = 2.0;
constexpr double ZOOM_STEP = 1.25;
// Los elementos sin dibujo (spawn, puertas, rocas) se marcan con un recuadro
const QColor UNDRAWN_ELEMENT(255, 235, 173, 120);
// Encima del piso; entre elementos, el de mas abajo tapa al de mas arriba
constexpr double ELEMENTS_Z = 1;
// El elemento seleccionado se ve semitransparente
constexpr double SELECTED_OPACITY = 0.5;
// El elemento por colocar va encima de los otros y debajo de los nombres de las areas; si no entra
// se tine de rojo
constexpr double GHOST_OPACITY = 0.5;
const QColor GHOST_BLOCKED(220, 30, 30);
// Las areas que se mueven: un recuadro claro, rojo si pisan otra
const QColor AREA_GHOST(255, 235, 173, 70);
const QColor AREA_GHOST_BLOCKED(220, 30, 30, 90);
// El borde y el nombre de cada area van encima de todo
constexpr double LABELS_Z = 1e9;
constexpr double GHOST_Z = LABELS_Z - 1;
constexpr double NAME_PADDING = 2;
// Lugar libre alrededor del mapa: para ver los bordes con zoom y para llevar areas afuera de lo
// que ya hay
constexpr int SCENE_MARGIN = 30 * TILE;
const QColor AREA_NAME(255, 235, 173);
const QColor NAME_BACKGROUND(45, 27, 16, 190);
// El mismo marron del fondo del menu
const QColor BACKGROUND(45, 27, 16);
// Las barras de scroll las pone la pantalla, para todo el editor
const char* const CANVAS_STYLE = R"(
    QGraphicsView {
        border: none;
    }
)";
const QColor AREA_BORDER(255, 235, 173, 160);
}  // namespace

MapCanvas::MapCanvas(MapEditing& editing, QWidget* parent):
        QGraphicsView(parent), editing(editing) {
    setScene(&scene);
    setBackgroundBrush(BACKGROUND);
    setStyleSheet(CANVAS_STYLE);
    scale(INITIAL_ZOOM, INITIAL_ZOOM);
    // Para que lleguen los movimientos sin boton apretado (el fantasma)
    viewport()->setMouseTracking(true);
    editing.onTileChanged([this](const maps::MapCell& cell) { updateTile(cell); });
    editing.onMapChanged([this]() { redraw(); });
}

void MapCanvas::setTool(Tool* active) { tool = active; }

void MapCanvas::showSelection(const std::optional<std::string>& elementId) {
    setElementOpacity(selection, 1);
    selection = elementId;
    setElementOpacity(selection, SELECTED_OPACITY);
}

void MapCanvas::showGhost(const std::optional<Ghost>& shown) {
    delete ghostItem;
    ghostItem = nullptr;
    ghost = shown;
    drawGhost();
}

void MapCanvas::showAreaGhost(const std::optional<AreaGhost>& shown) {
    for (QGraphicsItem* item: areaGhostItems) {
        delete item;
    }
    areaGhostItems.clear();
    areaGhost = shown;
    drawAreaGhost();
}

void MapCanvas::redraw() {
    const int horizontal = horizontalScrollBar()->value();
    const int vertical = verticalScrollBar()->value();
    scene.clear();
    ghostItem = nullptr;
    areaGhostItems.clear();
    areaTiles.clear();
    elementItems.clear();
    const maps::MapDefinition& map = editing.map();
    for (const maps::MapArea& area: map.areas) {
        addArea(area);
        for (const maps::MapElement& element: map.elements) {
            if (element.areaId == area.id) {
                addElement(area, element);
            }
        }
    }
    // Con lugar libre alrededor
    scene.setSceneRect(scene.itemsBoundingRect().adjusted(-SCENE_MARGIN, -SCENE_MARGIN,
                                                          SCENE_MARGIN, SCENE_MARGIN));
    horizontalScrollBar()->setValue(horizontal);
    verticalScrollBar()->setValue(vertical);
    // Las imagenes son nuevas: el seleccionado se vuelve a mostrar
    showSelection(selection);
    drawGhost();
    drawAreaGhost();
}

void MapCanvas::updateTile(const maps::MapCell& cell) {
    const std::optional<AreaCell> changed = cellAt(editing.map(), cell.x, cell.y);
    if (!changed) {
        return;
    }
    const maps::TileType tile = editing.map().areas[changed->area].tiles[changed->y][changed->x];
    areaTiles[changed->area][changed->y][changed->x]->setPixmap(sprites.tile(tile));
}

void MapCanvas::addArea(const maps::MapArea& area) {
    const QPointF origin(area.offsetX * TILE, area.offsetY * TILE);
    TileItems& items = areaTiles.emplace_back();
    for (int y = 0; y < static_cast<int>(area.tiles.size()); ++y) {
        std::vector<QGraphicsPixmapItem*>& row = items.emplace_back();
        for (int x = 0; x < static_cast<int>(area.tiles[y].size()); ++x) {
            QGraphicsPixmapItem* item = scene.addPixmap(sprites.tile(area.tiles[y][x]));
            item->setPos(origin + QPointF(x * TILE, y * TILE));
            row.push_back(item);
        }
    }
    // Un borde para ver donde termina cada area, y su nombre en la esquina de arriba a la
    // izquierda, sobre un fondo oscuro para que se lea sobre cualquier terreno. Van encima de todo.
    QGraphicsRectItem* border = scene.addRect(origin.x(), origin.y(), area.width * TILE,
                                              area.height * TILE, QPen(AREA_BORDER));
    border->setZValue(LABELS_Z);
    QGraphicsSimpleTextItem* name = scene.addSimpleText(QString::fromStdString(area.id));
    name->setBrush(AREA_NAME);
    name->setPos(origin + QPointF(NAME_PADDING, NAME_PADDING));
    name->setZValue(LABELS_Z + 1);
    const QRectF nameBox = name->sceneBoundingRect().adjusted(-NAME_PADDING, -NAME_PADDING,
                                                              NAME_PADDING, NAME_PADDING);
    scene.addRect(nameBox, QPen(Qt::NoPen), QBrush(NAME_BACKGROUND))->setZValue(LABELS_Z);
}

void MapCanvas::addElement(const maps::MapArea& area, const maps::MapElement& element) {
    QGraphicsItem* item = addElementImage(element.type, area.offsetX + element.x,
                                          area.offsetY + element.y + element.height, element.width,
                                          element.height);
    item->setToolTip(QString::fromStdString(element.id));
    elementItems[element.id] = item;
}

QGraphicsItem* MapCanvas::addElementImage(std::optional<maps::ElementType> type, int left,
                                          int bottom, int width, int height) {
    const int x = left * TILE;
    const int y = bottom * TILE;
    // Un mapa con errores puede traer elementos sin tipo: se abren igual para poder corregirlos
    std::optional<QPixmap> sprite = type ? sprites.element(*type) : std::nullopt;
    QGraphicsItem* item = nullptr;
    if (sprite) {
        // al ancho del elemento y apoyado en su base, el techo sobresale arriba
        QPixmap scaled = sprite->scaledToWidth(width * TILE);
        item = scene.addPixmap(scaled);
        item->setPos(x, y - scaled.height());
    } else {
        item = scene.addRect(x, y - height * TILE, width * TILE, height * TILE, QPen(Qt::NoPen),
                             QBrush(UNDRAWN_ELEMENT));
    }
    item->setZValue(ELEMENTS_Z + y);
    return item;
}

void MapCanvas::drawGhost() {
    if (!ghost) {
        return;
    }
    // La casilla del mouse es la de abajo a la izquierda de la base
    ghostItem = addElementImage(ghost->type, ghost->bottomLeft.x, ghost->bottomLeft.y + 1,
                                ghost->width, ghost->height);
    ghostItem->setZValue(GHOST_Z);
    ghostItem->setOpacity(GHOST_OPACITY);
    if (!ghost->fits) {
        auto* tint = new QGraphicsColorizeEffect();
        tint->setColor(GHOST_BLOCKED);
        // El item se queda con el efecto y lo borra
        ghostItem->setGraphicsEffect(tint);
    }
}

void MapCanvas::drawAreaGhost() {
    if (!areaGhost) {
        return;
    }
    const QColor fill = areaGhost->fits ? AREA_GHOST : AREA_GHOST_BLOCKED;
    for (const CellRect& rect: areaGhost->areas) {
        QGraphicsRectItem* item =
                scene.addRect(rect.x * TILE, rect.y * TILE, rect.width * TILE, rect.height * TILE,
                              QPen(AREA_BORDER), QBrush(fill));
        item->setZValue(GHOST_Z);
        areaGhostItems.push_back(item);
    }
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
    if (event->button() == Qt::MiddleButton) {
        lastDragPosition = event->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
    } else if (event->button() == Qt::LeftButton && tool) {
        tool->onPress(cellUnder(event->pos()));
    } else if (event->button() == Qt::RightButton && tool) {
        tool->onCancel();
    }
}

void MapCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::MiddleButton) {
        // El mapa sigue al mouse: las barras se mueven al reves de lo que se arrastra
        const QPoint delta = event->pos() - lastDragPosition;
        lastDragPosition = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    } else if ((event->buttons() & Qt::LeftButton) && tool) {
        tool->onMove(cellUnder(event->pos()));
    } else if (event->buttons() == Qt::NoButton && tool) {
        tool->onHover(cellUnder(event->pos()));
    }
}

void MapCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        viewport()->unsetCursor();
    } else if (event->button() == Qt::LeftButton && tool) {
        tool->onRelease(cellUnder(event->pos()));
    }
}

void MapCanvas::leaveEvent(QEvent* event) {
    showGhost(std::nullopt);
    QGraphicsView::leaveEvent(event);
}

void MapCanvas::setElementOpacity(const std::optional<std::string>& elementId, double opacity) {
    if (!elementId) {
        return;
    }
    // Un elemento recien borrado ya no tiene imagen
    auto item = elementItems.find(*elementId);
    if (item != elementItems.end()) {
        item->second->setOpacity(opacity);
    }
}

maps::MapCell MapCanvas::cellUnder(const QPoint& viewPosition) const {
    // floor para que -0.5 caiga en la casilla -1 y no en la 0
    const QPointF position = mapToScene(viewPosition);
    return {static_cast<int>(std::floor(position.x() / TILE)),
            static_cast<int>(std::floor(position.y() / TILE))};
}
