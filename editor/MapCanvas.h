#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "common/map/MapDefinition.h"
#include "editor/SpriteSheets.h"

class QGraphicsItem;
class QGraphicsPixmapItem;

class MapEditing;
class Tool;

// Dibuja el mapa que se esta editando con los sprites del juego, cada area en su offset. No cambia
// el mapa: convierte el mouse en una casilla del mapa general y se la pasa a la herramienta activa,
// y se redibuja cuando MapEditing avisa que algo cambio. Las coordenadas de la escena son pixeles
// del tileset: una casilla mide TILE_SIZE.
class MapCanvas: public QGraphicsView {
public:
    explicit MapCanvas(MapEditing& editing, QWidget* parent = nullptr);
    void setTool(Tool* tool);
    // Muestra semitransparente el elemento con ese id (y el anterior vuelve a verse normal).
    // Vacio: ninguno.
    void showSelection(const std::optional<std::string>& elementId);

protected:
    // Rueda: zoom centrado en el mouse
    void wheelEvent(QWheelEvent* event) override;
    // Click izquierdo: la herramienta activa. Arrastrar con la rueda apretada mueve el mapa.
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    // La imagen de cada casilla de un area, para cambiar solo la que se pinta
    using TileItems = std::vector<std::vector<QGraphicsPixmapItem*>>;

    // Arma la escena de nuevo; la vista queda donde estaba
    void redraw();
    void updateTile(const stardew::maps::MapCell& cell);
    void addArea(const stardew::maps::MapArea& area);
    void addElement(const stardew::maps::MapArea& area, const stardew::maps::MapElement& element);
    void setElementOpacity(const std::optional<std::string>& elementId, double opacity);
    stardew::maps::MapCell cellUnder(const QPoint& viewPosition) const;

    MapEditing& editing;
    Tool* tool = nullptr;
    QGraphicsScene scene;
    SpriteSheets sprites;
    // En el mismo orden que map.areas
    std::vector<TileItems> areaTiles;
    // La imagen de cada elemento por su id, para mostrar el seleccionado
    std::map<std::string, QGraphicsItem*> elementItems;
    std::optional<std::string> selection;
    // Donde estaba el mouse en el ultimo paso del arrastre con la rueda
    QPoint lastDragPosition;
};
