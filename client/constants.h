#ifndef CLIENT_CONSTANTS_H
#define CLIENT_CONSTANTS_H

#include <array>
#include <chrono>
#include <cstdint>

#include <SDL2pp/SDL2pp.hh>

#include "client/audio/sonido.h"
#include "client/gui/animador_sprites.h"
#include "common/lobby_dto.h"
#include "common/map/MapSprites.h"

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
inline const char* const MENSAJE_SIN_SONIDO = "Sin sonido: ";
// La funcion de SDL que fallo, para el mensaje de la excepcion
inline const char* const FUNCION_INICIO_AUDIO = "SDL_InitSubSystem";

// interfaz.cpp
inline const int ANCHO_VENTANA = 800;
inline const int ALTO_VENTANA = 600;
// En pixeles del tileset: la escala se elige para que entre mas o menos esto de alto (12 casillas
// y media), sea cual sea el tamaño de la ventana
inline const int ALTO_MUNDO_VISIBLE = 200;
// Mientras el jugador camina suena un paso cada tanto, alternando entre estos
inline const std::chrono::milliseconds INTERVALO_PASO(300);
inline constexpr std::array<Sonido, 4> SONIDOS_PASO = {Sonido::Paso1, Sonido::Paso2,
                                                       Sonido::Paso3, Sonido::Paso4};
// La hoja del granjero: cuadros de 16x32, una fila por orientacion (el costado mira a la derecha) y
// la caminata va quieto, pie izquierdo, quieto, pie derecho
inline const FormatoAnimacion FORMATO_PERSONAJE{16,
                                                32,
                                                {{Orientacion::Abajo, {0, false}},
                                                 {Orientacion::Arriba, {2, false}},
                                                 {Orientacion::Derecha, {1, false}},
                                                 {Orientacion::Izquierda, {1, true}}},
                                                {0, 1, 0, 2},
                                                std::chrono::duration<float>(0.15f)};
// Los brazos del granjero estan en la misma hoja, 6 cuadros a la derecha del cuerpo
inline constexpr int COLUMNAS_BRAZOS = 6;

// calculador_posicion.cpp
// Cada cuanto manda el servidor un Snapshot (ServerConstants::TICK_RATE = 30 por segundo)
inline const std::chrono::duration<float> DURACION_TICK_SERVIDOR(1.0f / 30);

// mapa.cpp
// En pixeles del tileset, sin escalar: el mismo de la tabla de sprites que comparte con el editor
inline const int TAMANO_TILE = stardew::maps::TILE_SIZE;
inline const char* const CARACTERES_WORLD_ID =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";

#endif
