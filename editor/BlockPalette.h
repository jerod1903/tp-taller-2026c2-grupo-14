#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"

class QButtonGroup;
class QHBoxLayout;
class QToolButton;
class QVBoxLayout;
class SpriteSheets;

// El panel de lo que se puede poner. Arriba, las areas nuevas (granja y pueblo; los pisos de la
// mina los crea la entrada). Abajo, los elementos, armados desde las reglas: una fila por tipo de
// area con una tarjeta (dibujo y nombre) por tipo de elemento. La fila que no entra en el ancho
// del panel se scrollea. Los portones, la salida y la escalera no estan: los pone el editor. Las
// tarjetas entran al grupo de botones de la pantalla, asi queda marcada la herramienta que se esta
// usando.
class BlockPalette: public QWidget {
    Q_OBJECT

public:
    BlockPalette(const stardew::maps::MapRequirements& requirements, SpriteSheets& sprites,
                 QButtonGroup* group, QWidget* parent = nullptr);

signals:
    void typeChosen(stardew::maps::ElementType type);
    void areaChosen(stardew::maps::AreaType type);

private:
    // Agrega al panel una fila con scroll horizontal, que aparece solo si las tarjetas no entran, y
    // devuelve donde van sus tarjetas
    QHBoxLayout* addRow(QVBoxLayout* panel);
    QToolButton* addCard(QHBoxLayout* cards, const QPixmap& icon, const QString& name,
                         QButtonGroup* group);
};
