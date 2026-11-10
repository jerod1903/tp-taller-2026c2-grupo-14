#pragma once

#include <QString>

#include "editor/GameDialog.h"

class QLineEdit;

// Pide el nombre de un mapa nuevo: el que muestra el lobby del juego. No se cambia despues.
class NameDialog: public GameDialog {
    Q_OBJECT

public:
    explicit NameDialog(QWidget* parent = nullptr);
    // El nombre sin espacios al principio ni al final
    QString name() const;

private:
    QLineEdit* nameInput;
};
