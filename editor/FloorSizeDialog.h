#pragma once

#include <QDialog>

#include "editor/Mine.h"

class MenuButton;
class QLineEdit;

// Pide el ancho y el alto de los pisos de la mina, entre 5 y 30 casillas, en un cartel de madera
// como los del menu del juego
class FloorSizeDialog: public QDialog {
    Q_OBJECT

public:
    explicit FloorSizeDialog(QWidget* parent = nullptr);
    FloorSize size() const;

private:
    // Aceptar solo con los dos numeros dentro del rango
    void updateAccept();

    QLineEdit* widthInput;
    QLineEdit* heightInput;
    MenuButton* acceptButton;
};
