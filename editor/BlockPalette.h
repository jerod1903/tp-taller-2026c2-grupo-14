#pragma once

#include <QWidget>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"

class QButtonGroup;
class QHBoxLayout;
class QVBoxLayout;
class SpriteSheets;

// El panel de los elementos que se pueden colocar, armado desde las reglas: una fila por tipo de
// area con una tarjeta (dibujo y nombre) por tipo de elemento; la fila que no entra en el ancho
// del panel se scrollea. Los portones, la entrada a la mina, la salida y la escalera no estan: los
// pone el editor. Las tarjetas entran al grupo de botones de la pantalla, asi queda marcada la
// herramienta que se esta usando.
class BlockPalette: public QWidget {
    Q_OBJECT

public:
    BlockPalette(const stardew::maps::MapRequirements& requirements, SpriteSheets& sprites,
                 QButtonGroup* group, QWidget* parent = nullptr);

signals:
    void typeChosen(stardew::maps::ElementType type);

private:
    // Agrega al panel una fila con scroll horizontal, que aparece solo si las tarjetas no entran, y
    // devuelve donde van sus tarjetas
    QHBoxLayout* addRow(QVBoxLayout* panel);
    void addCard(QHBoxLayout* cards, stardew::maps::ElementType type, SpriteSheets& sprites,
                 QButtonGroup* group);
};
