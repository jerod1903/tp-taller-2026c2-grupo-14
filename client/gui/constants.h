#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

#include <SDL2pp/SDL2pp.hh>

// interfaz.cpp
inline const int ANCHO_VENTANA = 800;
inline const int ALTO_VENTANA = 600;
inline const int ESCALA = 3;            // los assets son pixel art de 16x16
inline const Uint32 MS_POR_FRAME = 16;  // 60 fps aprox
// Provisorio: el id de este jugador lo va a asignar el servidor al entrar a la partida
inline const uint16_t ID_PROPIO = 1;
inline const char* const RUTA_SPRITE_PERSONAJE =
        "client/Assets/StardewValley_Character_Farmer Base.png";

// personaje.cpp
// Primer frame (arriba a la izquierda) del sheet, de 16x32
inline const SDL2pp::Rect FRAME(0, 0, 16, 32);

#endif
