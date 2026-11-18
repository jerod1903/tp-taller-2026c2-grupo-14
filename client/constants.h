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

inline constexpr int CANTIDAD_ARGUMENTOS = 4;
inline const char* const MUNDO_PRUEBA = "valle";
inline const char* const NOMBRE_PARTIDA_PRUEBA = "prueba";
inline constexpr CharacterAppearance APARIENCIA_PRUEBA{0, 0, 0, 0, 0};

inline const char* const MENSAJE_SIN_SONIDO = "Sin sonido: ";
inline const char* const FUNCION_INICIO_AUDIO = "SDL_InitSubSystem";

inline const std::chrono::duration<float> DURACION_TRANSICION(0.5f);
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

// hud.cpp: los carteles con el dia y la hora, el oro y la energia. Las medidas van en pixeles de
// los assets; el HUD se dibuja a una escala entera
inline constexpr int HORA_INICIO_DIA = 6;      // el minuto 0 del dia son las 6:00
inline constexpr int PASO_MINUTOS_RELOJ = 10;  // la hora se muestra de a 10 minutos
inline const char* const PREFIJO_DIA = "Día ";
inline constexpr int TAMANO_FUENTE_HUD = 18;
inline constexpr int MARGEN_HUD = 4;
// botones_madera_enlazados.png: tres carteles de 96x32 (con la sombra), uno cada 40 (con las
// cadenas); se usan los dos de arriba. El texto se centra en la cara, sin la sombra de abajo
inline constexpr int ANCHO_CARTEL = 96;
inline constexpr int ALTO_CARTEL = 32;
inline constexpr int ALTO_CARA_CARTEL = 28;
inline constexpr int PASO_CARTELES = 40;
inline constexpr int CANTIDAD_CARTELES = 2;
inline constexpr int SEPARACION_CARTEL_ORO = 4;
inline constexpr int RELLENO_CARTEL = 12;  // de la punta redonda al icono o al texto
// input_madera_panel.png: el marco de la barra de energia, que se estira sin tocar el borde
inline constexpr int BORDE_MARCO = 9;
inline constexpr int ANCHO_BARRA_ENERGIA = 26;
inline constexpr int ALTO_BARRA_ENERGIA = 80;
inline constexpr int PORCENTAJE_ENERGIA_MEDIA = 50;
inline constexpr int PORCENTAJE_ENERGIA_BAJA = 20;
inline const char* const ETIQUETA_ENERGIA = "E";
inline constexpr SDL_Color COLOR_TEXTO_HUD = {255, 240, 210, 255};
inline constexpr SDL_Color COLOR_SOMBRA_HUD = {43, 28, 35, 255};
inline constexpr SDL_Color COLOR_ENERGIA_ALTA = {120, 200, 60, 255};
inline constexpr SDL_Color COLOR_ENERGIA_MEDIA = {230, 190, 50, 255};
inline constexpr SDL_Color COLOR_ENERGIA_BAJA = {200, 60, 50, 255};

// globo.cpp: el globito con el emote arriba de un personaje
inline constexpr int SEPARACION_GLOBO = 1;
inline constexpr int RELLENO_GLOBO = 2;
inline constexpr int ALTO_PALITO = 3;
inline constexpr SDL_Color COLOR_FONDO_GLOBO = {255, 255, 255, 255};
inline constexpr SDL_Color COLOR_BORDE_GLOBO = {38, 43, 68, 255};

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

inline const Vistas VISTAS_PELO = {{Orientacion::Abajo, {0, false}},
                                   {Orientacion::Derecha, {1, false}},
                                   {Orientacion::Izquierda, {1, true}},
                                   {Orientacion::Arriba, {2, false}}};
inline const Vistas VISTAS_CUATRO = {{Orientacion::Abajo, {0, false}},
                                     {Orientacion::Derecha, {1, false}},
                                     {Orientacion::Izquierda, {2, false}},
                                     {Orientacion::Arriba, {3, false}}};
inline constexpr int BASE_HEXADECIMAL = 16;
inline constexpr uint32_t MASCARA_ALFA = 0xFF000000;
inline constexpr uint32_t MASCARA_COLOR = 0x00FFFFFF;

inline const int TAMANO_TILE = stardew::maps::TILE_SIZE;
inline const char* const CARACTERES_WORLD_ID =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";

#endif
