#pragma once

#include <QDialog>
#include <QString>

#include "editor/MapHelper.h"

class MenuButton;
class QLineEdit;

// Que tamano se pide: el titulo, una aclaracion, el rango de cada lado y el valor inicial
struct SizeRequest {
    QString title;
    QString note;
    int minSide;
    int maxSide;
    AreaSize initial;
};

// Pide un ancho y un alto en casillas (los pisos de la mina, una granja o un pueblo nuevos), en un
// cartel de madera como los del menu del juego
class SizeDialog: public QDialog {
    Q_OBJECT

public:
    explicit SizeDialog(const SizeRequest& request, QWidget* parent = nullptr);
    AreaSize size() const;

private:
    // Aceptar solo con los dos numeros dentro del rango
    void updateAccept();

    QLineEdit* widthInput;
    QLineEdit* heightInput;
    MenuButton* acceptButton;
};
