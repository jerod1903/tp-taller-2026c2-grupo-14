#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

#include <SDL2pp/SDL2pp.hh>

// interfaz.cpp
inline const int ANCHO_VENTANA = 800;
inline const int ALTO_VENTANA = 600;
inline const int ESCALA = 3;            // los assets son pixel art de 16x16
inline const Uint32 MS_POR_FRAME = 16;  // 60 fps aprox
inline const char* const RUTA_SPRITE_PERSONAJE =
        "Assets/Characters/StardewValley/StardewValley_Character_Farmer Base.png";

// personaje.cpp
// Primer frame (arriba a la izquierda) del sheet, de 16x32
inline const SDL2pp::Rect FRAME(0, 0, 16, 32);

// mapa.cpp
inline const int TAMANO_TILE = 16;  // en pixeles del tileset, sin escalar
inline const char* const RUTA_TILESET_CAVERNA = "Assets/Caverns/Environment/Dungeon_Tileset.png";
inline const char* const RUTA_TILESET_EXTERIOR = "Assets/Misc/town.png";
inline const char* const CARACTERES_WORLD_ID =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
// Casillas (columna, fila) dentro del tileset de la caverna
inline const SDL2pp::Point TILE_PARED(1, 0);
inline const SDL2pp::Point TILE_PISO(6, 0);
// Casillas (columna, fila) dentro del tileset del exterior
inline const SDL2pp::Point TILE_PASTO(2, 2);
inline const SDL2pp::Point TILE_TIERRA(9, 8);
inline const SDL2pp::Point TILE_CAMINO(10, 7);
inline const SDL2pp::Point TILE_TIERRA_ARADA(12, 7);
inline const SDL2pp::Point TILE_AGUA(12, 19);
inline const SDL2pp::Point TILE_ARENA(8, 1);
inline const SDL2pp::Point TILE_PIEDRA(13, 16);
inline const SDL2pp::Point TILE_PISO_MADERA(2, 9);

#endif
