#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <vector>

#include "common/map/MapDefinition.h"
#include "editor/SpriteSheets.h"

class QGraphicsPixmapItem;

// Dibuja el mapa entero con los sprites del juego, cada area en su offset, y deja pintar con el
// pincel. Las coordenadas de la escena son pixeles del tileset: una casilla mide TILE_SIZE.
class MapCanvas: public QGraphicsView {
public:
    explicit MapCanvas(QWidget* parent = nullptr);

    // Edita el mapa en el lugar: tiene que vivir mientras se muestre
    void showMap(stardew::maps::MapDefinition& map);
    // Herramientas del click izquierdo: pintar con un tile, o poner y quitar pasajes
    void setBrush(stardew::maps::TileType tile);
    void usePassageTool();

protected:
    // Rueda: zoom centrado en el mouse
    void wheelEvent(QWheelEvent* event) override;
    // Click izquierdo (y arrastrar): pinta casillas con el pincel. Arrastrar con la rueda apretada
    // mueve el mapa.
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    // Un area con la imagen de cada casilla, para cambiar solo la que se pinta
    struct DrawnArea {
        stardew::maps::MapArea* area;
        std::vector<std::vector<QGraphicsPixmapItem*>> tiles;
    };

    void addArea(stardew::maps::MapArea& area);
    void addElement(const stardew::maps::MapArea& area, const stardew::maps::MapElement& element);
    void paintAt(const QPoint& viewPosition);
    void togglePassageAt(const QPoint& viewPosition);

    stardew::maps::MapDefinition* map = nullptr;
    QGraphicsScene scene;
    SpriteSheets sprites;
    std::vector<DrawnArea> areas;
    stardew::maps::TileType brush = stardew::maps::TileType::Grass;
    bool passageTool = false;
    // Donde estaba el mouse en el ultimo paso del arrastre con la rueda
    QPoint lastDragPosition;
};
