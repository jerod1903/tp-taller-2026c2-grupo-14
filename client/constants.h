#ifndef CLIENT_CONSTANTS_H
#define CLIENT_CONSTANTS_H

#include <chrono>
#include <cstdint>
#include <map>
#include <utility>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/audio/sonido.h"
#include "client/gui/sprites/animador_sprites.h"
#include "client/gui/inventario/objeto.h"
#include "client/gui/sprites/capa.h"
#include "common/character/CharacterSheets.h"
#include "common/lobby_dto.h"
#include "common/map/MapSprites.h"

inline constexpr int CANTIDAD_ARGUMENTOS = 4;
inline const char* const MUNDO_PRUEBA = "valle";
inline const char* const NOMBRE_PARTIDA_PRUEBA = "prueba";
inline constexpr CharacterAppearance APARIENCIA_PRUEBA{0, 0, 0, 0, 0};

inline const std::chrono::milliseconds DURACION_FRAME(16);

inline constexpr int CANALES_AUDIO = 16;
inline constexpr int TAMANO_BUFFER_AUDIO = 1024;
inline const char* const MENSAJE_SIN_SONIDO = "Sin sonido: ";
inline const char* const FUNCION_INICIO_AUDIO = "SDL_InitSubSystem";

inline const int ANCHO_VENTANA = 800;
inline const int ALTO_VENTANA = 600;
inline const int ALTO_MUNDO_VISIBLE = 200;
inline const std::chrono::milliseconds INTERVALO_PASO(300);
inline const FormatoAnimacion FORMATO_PERSONAJE{stardew::character::FRAME_SIZE.x,
                                                stardew::character::FRAME_SIZE.y,
                                                {{Orientacion::Abajo, {0, false}},
                                                 {Orientacion::Arriba, {2, false}},
                                                 {Orientacion::Derecha, {1, false}},
                                                 {Orientacion::Izquierda, {1, true}}},
                                                {0, 1, 0, 2},
                                                std::chrono::duration<float>(0.15f),
                                                {{1, 1}, {2, 1}}};

inline const FormatoAnimacion FORMATO_VACA{32,
                                           32,
                                           {{Orientacion::Izquierda, {0, false}},
                                            {Orientacion::Derecha, {0, true}},
                                            {Orientacion::Abajo, {1, false}},
                                            {Orientacion::Arriba, {2, false}}},
                                           {0, 1, 2, 3},
                                           std::chrono::duration<float>(0.15f)};
inline const FormatoAnimacion FORMATO_GALLINA{16,
                                              16,
                                              {{Orientacion::Izquierda, {0, false}},
                                               {Orientacion::Derecha, {1, false}},
                                               {Orientacion::Abajo, {0, false}},
                                               {Orientacion::Arriba, {1, false}}},
                                              {0, 1, 2, 3},
                                              std::chrono::duration<float>(0.15f)};

inline constexpr int CANTIDAD_CASILLEROS = 9;
inline constexpr int TAMANO_CASILLERO = 18;
inline constexpr int SEPARACION_CASILLEROS = 1;
inline constexpr int MARGEN_BARRA = 3;
inline constexpr SDL_Color COLOR_FONDO_CASILLERO = {60, 40, 30, 220};
inline constexpr SDL_Color COLOR_BORDE_CASILLERO = {150, 110, 70, 255};
inline constexpr SDL_Color COLOR_SELECCIONADO = {250, 210, 60, 255};
inline constexpr SDL_Color COLOR_NUMERO = {255, 255, 255, 255};
inline constexpr SDL_Color COLOR_SOMBRA_NUMERO = {0, 0, 0, 255};
inline constexpr int ICONO_MAXIMO = 16;
inline const std::map<Objeto, SDL2pp::Rect> RECORTES_ICONOS = {
        {Objeto::Cama, SDL2pp::Rect(626, 152, 32, 56)},
};
inline const std::map<Objeto, SDL_Color> COLORES_SIN_ICONO = {
        {Objeto::Regadera, {70, 130, 200, 255}},
};
inline constexpr int MARGEN_SIN_ICONO = 5;
inline constexpr int ANCHO_DIGITO = 3;
inline constexpr int ALTO_DIGITO = 5;
inline constexpr std::array<std::array<const char*, 5>, 10> DIGITOS = {{
        {"###", "#.#", "#.#", "#.#", "###"},
        {".#.", "##.", ".#.", ".#.", "###"},
        {"###", "..#", "###", "#..", "###"},
        {"###", "..#", "###", "..#", "###"},
        {"#.#", "#.#", "###", "..#", "..#"},
        {"###", "#..", "###", "..#", "###"},
        {"###", "#..", "###", "#.#", "###"},
        {"###", "..#", "..#", "..#", "..#"},
        {"###", "#.#", "###", "#.#", "###"},
        {"###", "#.#", "###", "..#", "###"},
}};

inline const Vistas VISTAS_PELO = {
        {Orientacion::Abajo, {0, false}},
        {Orientacion::Derecha, {1, false}},
        {Orientacion::Izquierda, {1, true}},
        {Orientacion::Arriba, {2, false}}};
inline const Vistas VISTAS_CUATRO = {
        {Orientacion::Abajo, {0, false}},
        {Orientacion::Derecha, {1, false}},
        {Orientacion::Izquierda, {2, false}},
        {Orientacion::Arriba, {3, false}}};
inline constexpr int BASE_HEXADECIMAL = 16;
inline constexpr uint32_t MASCARA_ALFA = 0xFF000000;
inline constexpr uint32_t MASCARA_COLOR = 0x00FFFFFF;

inline const std::chrono::duration<float> DURACION_TICK_SERVIDOR(1.0f / 30);

inline const int TAMANO_TILE = stardew::maps::TILE_SIZE;
inline const char* const CARACTERES_WORLD_ID =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";

#endif
