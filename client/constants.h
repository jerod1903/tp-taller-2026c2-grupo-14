#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

// interfaz_juego.cpp
inline const int ANCHO_VENTANA = 800;
inline const int ALTO_VENTANA = 600;
inline const int ESCALA = 3;  // los assets son pixel art de 16x16
inline const int VELOCIDAD_MOV = 4;
inline const Uint32 MS_POR_FRAME = 16;  // 60 fps aprox

// caverna.cpp
inline const char* const RUTA_TILESET_CAVERNA =
        "client/Assets/Caverns/Environment/Dungeon_Tileset.png";
inline const int TAMANO_TILE_ORIGINAL = 16;
inline const char PARED = '#';
inline const char PISO = '.';
inline const char INICIO = 'P';

inline const std::vector<std::string> MAPA_CAVERNA = {
         "#############################",
         "##########.........##########",
         "##########.........##########",
         "#......###.........###......#",
         "#......######...######......#",
         "#......######...######......#",
         "###..#####.........#####..###",
         "###..#####.........#####..###",
         "#...........................#",
         "#......###.........###......#",
         "#......######..#######......#",
         "###..########..#########..###",
         "##........###..###.........##",
         "##........##....##.........##",
         "##........##....##.........##",
         "######..#####..#####..#######",
         "######..#####..#####..#######",
         "###......##.P....##......####",
         "###......##......##......####",
         "#############################",
};

// coords d los tiles en Dungeon_Tileset.png
inline const SDL2pp::Point TILE_PARED(1, 0);
inline const SDL2pp::Point TILE_PISO(6, 0);

// personaje.cpp
inline const char* const RUTA_SPRITE_PERSONAJE =
        "client/Assets/Characters/StardewValley/StardewValley_Character_Farmer Base.png";

// Primer frame (arriba a la izquierda) del sheet, de 16x32
inline const SDL2pp::Rect FRAME(0, 0, 16, 32);

// Pies del personaje, relativo al frame (en pixeles sin escalar)
inline const SDL2pp::Rect PIES(3, 26, 10, 6);

// armador_acciones.cpp
// Primer byte de cada accion: que quiere hacer el usuario
inline const uint8_t ACCION_MOVER = 0x01;

// Segundo byte de ACCION_MOVER: hacia donde
inline const uint8_t DIRECCION_ARRIBA = 0x01;
inline const uint8_t DIRECCION_ABAJO = 0x02;
inline const uint8_t DIRECCION_IZQUIERDA = 0x03;
inline const uint8_t DIRECCION_DERECHA = 0x04;
inline const uint8_t DIRECCION_ARRIBA_IZQUIERDA = 0x05;
inline const uint8_t DIRECCION_ARRIBA_DERECHA = 0x06;
inline const uint8_t DIRECCION_ABAJO_IZQUIERDA = 0x07;
inline const uint8_t DIRECCION_ABAJO_DERECHA = 0x08;

#endif
