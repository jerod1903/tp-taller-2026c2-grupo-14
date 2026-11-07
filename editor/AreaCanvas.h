#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <vector>

#include "common/map/MapDefinition.h"
#include "editor/SpriteSheets.h"

class QGraphicsPixmapItem;

// Dibuja un area del mapa con los sprites del juego y la deja pintar con el pincel. Las
// coordenadas de la escena son pixeles del tileset: una casilla mide TILE_SIZE.
class AreaCanvas: public QGraphicsView {
public:
    explicit AreaCanvas(QWidget* parent = nullptr);

    // Edita el area en el lugar: el mapa tiene que vivir mientras se muestre
    void showArea(stardew::maps::MapDefinition& map, stardew::maps::MapArea& area);
    void setBrush(stardew::maps::TileType tile);

protected:
    // Rueda: zoom centrado en el mouse
    void wheelEvent(QWheelEvent* event) override;
    // Click izquierdo (y arrastrar): pinta casillas con el pincel
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void addElement(const stardew::maps::MapElement& element);
    void paintAt(const QPoint& viewPosition);

    QGraphicsScene scene;
    SpriteSheets sprites;
    stardew::maps::MapArea* area = nullptr;
    // Una por casilla, para cambiar solo la que se pinta
    std::vector<std::vector<QGraphicsPixmapItem*>> tileItems;
    stardew::maps::TileType brush = stardew::maps::TileType::Grass;
};
