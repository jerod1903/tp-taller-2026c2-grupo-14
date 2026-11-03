#pragma once

#include <QString>

// Medidas compartidas por todas las pantallas del menu.
namespace Theme {
constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 720;
constexpr int SCREEN_MARGIN_H = 50;
constexpr int SCREEN_MARGIN_V = 12;
constexpr int SCREEN_SPACING = 15;
// Igual al alto del sprite de boton a 2x (2 tapas de 32 px + tramo recto): asi no se deforma.
constexpr int BUTTON_MIN_HEIGHT = 84;
constexpr int MENU_COLUMN_WIDTH = 320;
// Botones secundarios (volver, confirmar, jugar). Tienen su propio sprite de este ancho.
constexpr int SMALL_BUTTON_WIDTH = 200;
// Boton "Volver": 70% del boton normal.
constexpr int BACK_BUTTON_WIDTH = 140;
constexpr int BACK_BUTTON_HEIGHT = 59;
// Alto reservado para mensajes de error de hasta dos lineas, asi el layout no salta.
constexpr int ERROR_LABEL_HEIGHT = 36;
}  // namespace Theme

// Centraliza la hoja de estilos para que los widgets no definan colores por su cuenta.
class StyleManager {
public:
    static QString globalStyleSheet();
};
