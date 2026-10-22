#pragma once

#include <QString>

// Medidas compartidas por todas las pantallas del menu.
namespace Theme {
constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 720;
constexpr int SCREEN_MARGIN_H = 50;
constexpr int SCREEN_MARGIN_V = 40;
constexpr int SCREEN_SPACING = 15;
constexpr int BUTTON_MIN_HEIGHT = 42;
constexpr int MENU_COLUMN_WIDTH = 320;
}  // namespace Theme

// Centraliza la hoja de estilos para que los widgets no definan colores por su cuenta.
class StyleManager {
public:
    static QString globalStyleSheet();
};
