#pragma once

#include <QPushButton>

// Boton del menu: el estilo sale de la hoja global, aca solo se fija tamano y cursor.
class MenuButton: public QPushButton {
public:
    explicit MenuButton(const QString& text, QWidget* parent = nullptr);
    // Pasa al ancho chico (volver, confirmar, jugar) con su propio sprite.
    void setSmall();
    // Boton "Volver" chico, con el sprite de 140x59.
    void setBack();
};
