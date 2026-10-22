#pragma once

#include <QPushButton>

// Boton del menu: el estilo sale de la hoja global, aca solo se fija tamano y cursor.
class MenuButton: public QPushButton {
public:
    explicit MenuButton(const QString& text, QWidget* parent = nullptr);
};
