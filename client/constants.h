#ifndef CLIENT_CONSTANTS_H
#define CLIENT_CONSTANTS_H

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/audio/sonido.h"
#include "client/gui/sprites/animador_sprites.h"
#include "client/gui/sprites/capa.h"
#include "common/character/CharacterSheets.h"
#include "common/game_dto.h"
#include "common/lobby_dto.h"
#include "common/map/MapSprites.h"

// main_juego.cpp
inline constexpr int CANTIDAD_ARGUMENTOS = 4;  // programa, host, puerto y usuario
inline const char* const MUNDO_PRUEBA = "valle";
inline const char* const NOMBRE_PARTIDA_PRUEBA = "prueba";
inline constexpr CharacterAppearance APARIENCIA_PRUEBA{0, 0, 0, 0, 0};

// reproductor_sonidos.cpp
inline const char* const MENSAJE_SIN_SONIDO = "Sin sonido: ";
// La funcion de SDL que fallo, para el mensaje de la excepcion
inline const char* const FUNCION_INICIO_AUDIO = "SDL_InitSubSystem";

// interfaz.cpp
// La hoja del granjero: una fila por orientacion (el costado mira a la derecha) y la caminata va
// quieto, pie izquierdo, quieto, pie derecho. En los pasos la cabeza baja un pixel
inline const FormatoAnimacion FORMATO_PERSONAJE{stardew::character::FRAME_SIZE.x,
                                                stardew::character::FRAME_SIZE.y,
                                                {{Orientacion::Abajo, {0, false}},
                                                 {Orientacion::Arriba, {2, false}},
                                                 {Orientacion::Derecha, {1, false}},
                                                 {Orientacion::Izquierda, {1, true}}},
                                                {0, 1, 0, 2},
                                                std::chrono::duration<float>(0.15f),
                                                {{1, 1}, {2, 1}}};

// La vaca: cuadros de 32x32; costado (mirando a la izquierda), frente y espalda
inline const FormatoAnimacion FORMATO_VACA{32,
                                           32,
                                           {{Orientacion::Izquierda, {0, false}},
                                            {Orientacion::Derecha, {0, true}},
                                            {Orientacion::Abajo, {1, false}},
                                            {Orientacion::Arriba, {2, false}}},
                                           {0, 1, 2, 3},
                                           std::chrono::duration<float>(0.15f)};
// La gallina: cuadros de 16x16; solo trae los dos costados (izquierda y derecha)
inline const FormatoAnimacion FORMATO_GALLINA{16,
                                              16,
                                              {{Orientacion::Izquierda, {0, false}},
                                               {Orientacion::Derecha, {1, false}},
                                               {Orientacion::Abajo, {0, false}},
                                               {Orientacion::Arriba, {1, false}}},
                                              {0, 1, 2, 3},
                                              std::chrono::duration<float>(0.15f)};

// game_loop.cpp, interfaz.cpp y panel_chat.cpp: el chat
inline constexpr std::size_t MAX_MENSAJES_GUARDADOS = 50;

inline constexpr int TAMANO_FUENTE_CHAT = 16;
inline constexpr std::size_t MAX_LINEAS_CHAT = 6;
inline const std::chrono::seconds DURACION_MENSAJE_CHAT(10);
inline constexpr int PORCENTAJE_ANCHO_CHAT = 45;
inline constexpr int MARGEN_CHAT = 12;
inline constexpr int DISTANCIA_CHAT_ABAJO = 90;
inline constexpr int RELLENO_CHAT = 4;
inline const char* const PROMPT_CHAT = "> ";
inline const char* const CURSOR_CHAT = "_";
inline const char* const MARCA_PRIVADO = " (privado)";
inline constexpr SDL_Color COLOR_FONDO_CHAT = {20, 15, 10, 170};
inline constexpr SDL_Color COLOR_BORRADOR_CHAT = {250, 210, 60, 255};
inline const std::map<ChatKind, SDL_Color> COLORES_CHAT = {
        {ChatKind::Public, {255, 255, 255, 255}},
        {ChatKind::Private, {255, 150, 200, 255}},
        {ChatKind::System, {170, 170, 170, 255}},
};

// globo.cpp: el globito con el emote arriba de un personaje
inline constexpr int SEPARACION_GLOBO = 1;
inline constexpr int RELLENO_GLOBO = 2;
inline constexpr int ALTO_PALITO = 3;
inline constexpr SDL_Color COLOR_FONDO_GLOBO = {255, 255, 255, 255};
inline constexpr SDL_Color COLOR_BORDE_GLOBO = {38, 43, 68, 255};

// barra_inventario.cpp
inline constexpr int TAMANO_CASILLERO = 18;
inline constexpr int SEPARACION_CASILLEROS = 1;
inline constexpr int MARGEN_BARRA = 3;
inline constexpr SDL_Color COLOR_FONDO_CASILLERO = {60, 40, 30, 220};
inline constexpr SDL_Color COLOR_BORDE_CASILLERO = {150, 110, 70, 255};
inline constexpr SDL_Color COLOR_SELECCIONADO = {250, 210, 60, 255};
inline constexpr SDL_Color COLOR_NUMERO = {255, 255, 255, 255};
inline constexpr SDL_Color COLOR_SOMBRA_NUMERO = {0, 0, 0, 255};
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

// constructor_capas.cpp
// Que vista de cada hoja va con cada orientacion: (numero de vista, si se espeja). El pelo trae un
// solo costado; la camisa y el sombrero traen los dos
inline const Vistas VISTAS_PELO = {{Orientacion::Abajo, {0, false}},
                                   {Orientacion::Derecha, {1, false}},
                                   {Orientacion::Izquierda, {1, true}},
                                   {Orientacion::Arriba, {2, false}}};
inline const Vistas VISTAS_CUATRO = {{Orientacion::Abajo, {0, false}},
                                     {Orientacion::Derecha, {1, false}},
                                     {Orientacion::Izquierda, {2, false}},
                                     {Orientacion::Arriba, {3, false}}};
inline constexpr int BASE_HEXADECIMAL = 16;
// Partes de un pixel ARGB8888, para repintar las mangas sin tocar la transparencia
inline constexpr uint32_t MASCARA_ALFA = 0xFF000000;
inline constexpr uint32_t MASCARA_COLOR = 0x00FFFFFF;

// mapa.cpp
// En pixeles del tileset, sin escalar: el mismo de la tabla de sprites que comparte con el editor
inline const int TAMANO_TILE = stardew::maps::TILE_SIZE;
inline const char* const CARACTERES_WORLD_ID =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";

#endif
