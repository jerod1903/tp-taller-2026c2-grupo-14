#pragma once

#include "client/menu/screens/BaseScreen.h"

// Pantalla provisoria para probar la navegacion mientras la pantalla real no existe.
// Se borra cuando esten todas las pantallas.
class PlaceholderScreen: public BaseScreen {
public:
    explicit PlaceholderScreen(const QString& title, QWidget* parent = nullptr);
};
