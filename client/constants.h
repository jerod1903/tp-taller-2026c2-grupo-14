#ifndef CLIENT_CONSTANTS_H
#define CLIENT_CONSTANTS_H

#include <chrono>
#include <cstdint>

#include <SDL2pp/SDL2pp.hh>

#include "common/lobby_dto.h"

// main_juego.cpp
inline constexpr int CANTIDAD_ARGUMENTOS = 4;  // programa, host, puerto y usuario
inline const char* const MUNDO_PRUEBA = "valle";
inline const char* const NOMBRE_PARTIDA_PRUEBA = "prueba";
inline constexpr CharacterAppearance APARIENCIA_PRUEBA{0, 0, 0, 0, 0};

// game_loop.cpp
inline const std::chrono::milliseconds DURACION_FRAME(16);  // 60 fps aprox

// reproductor_sonidos.cpp
inline constexpr int CANALES_AUDIO = 16;  // cuantos efectos pueden sonar a la vez
// Muestras por bloque: mas chico suena antes (1024 a 44100 Hz son 23 ms), mas grande corta menos
inline constexpr int TAMANO_BUFFER_AUDIO = 1024;

// interfaz.cpp
inline const int ANCHO_VENTANA = 800;
inline const int ALTO_VENTANA = 600;
// En pixeles del tileset: la escala se elige para que entre mas o menos esto de alto (12 casillas
// y media), sea cual sea el tamaño de la ventana
inline const int ALTO_MUNDO_VISIBLE = 200;

// personaje.cpp
// Primer frame (arriba a la izquierda) del sheet, de 16x32
inline const SDL2pp::Rect FRAME(0, 0, 16, 32);
// Cada cuanto manda el servidor un Snapshot (ServerConstants::TICK_RATE = 30 por segundo)
inline const std::chrono::duration<float> DURACION_TICK_SERVIDOR(1.0f / 30);

// mapa.cpp
inline const int TAMANO_TILE = 16;  // en pixeles del tileset, sin escalar
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
// Recortes de los elementos del mapa dentro de su imagen
inline const SDL2pp::Rect RECORTE_CABANA(0, 0, 80, 112);
inline const SDL2pp::Rect RECORTE_ALMACEN(80, 176, 160, 144);  // el de Pierre
inline const SDL2pp::Rect RECORTE_CENTRO_COMUNITARIO(0, 640, 192, 160);
inline const SDL2pp::Rect FRAME_NPC(0, 0, 16, 32);  // mirando hacia abajo

#endif
