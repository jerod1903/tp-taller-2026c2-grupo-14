#pragma once

#include <QString>

#include "common/map/MapRequirements.h"
#include "editor/GameDialog.h"

class QLineEdit;

// Pide el nombre de un mapa nuevo: el que muestra el lobby del juego. No se cambia despues. Antes
// cuenta lo que tiene que tener el mapa para ser valido, segun las reglas.
class NameDialog: public GameDialog {
    Q_OBJECT

public:
    explicit NameDialog(const stardew::maps::MapRequirements& requirements,
                        QWidget* parent = nullptr);
    // El nombre sin espacios al principio ni al final
    QString name() const;

private:
    QLineEdit* nameInput;
};
