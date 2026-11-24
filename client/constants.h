#ifndef CLIENT_CONSTANTS_H
#define CLIENT_CONSTANTS_H

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/audio/sonido.h"
#include "client/gui/menu-pausa/menu_pausa.h"
#include "client/gui/rutas.h"
#include "client/gui/sprites/animador_sprites.h"
#include "client/gui/sprites/capa.h"
#include "client/gui/sprites/pose.h"
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
inline const std::chrono::duration<float> DURACION_GOLPE_ESPADA(0.25f);
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

// Las entidades que tienen un PNG por pose (lo que estén haciendo). Para sumar un enemigo alcanza
// con agregarlo acá.
// El esqueleto: una tira de cuadros de 32x32 por pose. Pegar y doler duran lo mismo que en el
// servidor (0.6 y 0.4 segundos, en Skeleton), así la animación entra completa.
inline constexpr int LADO_CUADRO_ESQUELETO = 32;
// El vampiro y la vampira: las mismas tiras de 32x32, cada uno con su tinte (TINTES). Pegar y
// doler duran lo mismo que en el servidor (0.8 y 0.4 segundos, en Vampire).
inline constexpr int LADO_CUADRO_VAMPIRO = 32;
inline const std::map<Pose, HojaDePose> HOJAS_VAMPIRO = {
        {Pose::Quieto,
         {RUTA_VAMPIRO_QUIETO, formato_tira(LADO_CUADRO_VAMPIRO, LADO_CUADRO_VAMPIRO, 6,
                                            std::chrono::duration<float>(0.15f))}},
        {Pose::Caminando,
         {RUTA_VAMPIRO_CAMINANDO, formato_tira(LADO_CUADRO_VAMPIRO, LADO_CUADRO_VAMPIRO, 8,
                                               std::chrono::duration<float>(0.1f))}},
        {Pose::Atacando,
         {RUTA_VAMPIRO_ATACANDO, formato_tira(LADO_CUADRO_VAMPIRO, LADO_CUADRO_VAMPIRO, 16,
                                              std::chrono::duration<float>(0.8f / 16))}},
        {Pose::Herido,
         {RUTA_VAMPIRO_HERIDO, formato_tira(LADO_CUADRO_VAMPIRO, LADO_CUADRO_VAMPIRO, 5,
                                            std::chrono::duration<float>(0.4f / 5))}},
        {Pose::Muerto,
         {RUTA_VAMPIRO_MUERTO, formato_tira(LADO_CUADRO_VAMPIRO, LADO_CUADRO_VAMPIRO, 14,
                                            std::chrono::duration<float>(0.08f), false)}},
};
// El orco: cuadros de 100x100 con el cuerpo chico y los pies en la fila 57. Se usan solo las
// ALTO_CUADRO_ORCO filas de arriba, así los pies quedan abajo del cuadro como en los demás. Pegar
// y doler duran lo mismo que en el servidor (0.6 y 0.4 segundos, en Orc).
inline constexpr int ANCHO_CUADRO_ORCO = 100;
inline constexpr int ALTO_CUADRO_ORCO = 58;
// La calavera: una tira de 4 cuadros de 16x16 (los de Idle/v2), la misma en todas las poses. La
// verde, teñida (TINTES).
inline constexpr int LADO_CUADRO_CALAVERA = 16;
inline const std::map<Pose, HojaDePose> HOJAS_CALAVERA =
        hojas_de({RUTA_CALAVERA, formato_tira(LADO_CUADRO_CALAVERA, LADO_CUADRO_CALAVERA, 4,
                                              std::chrono::duration<float>(0.15f))});
// La llama: fire_skull.png achicada a su tamaño real (9x11), un solo cuadro. La verde, teñida.
inline constexpr int ANCHO_LLAMA = 9;
inline constexpr int ALTO_LLAMA = 11;
inline const std::map<Pose, HojaDePose> HOJAS_LLAMA = hojas_de(
        {RUTA_LLAMA, formato_tira(ANCHO_LLAMA, ALTO_LLAMA, 1, std::chrono::duration<float>(1.0f))});
// Las trampas: quietas, un solo cuadro (pinches escondidos, la boquilla del lanzallamas); activas,
// la animación de activarse. El servidor manda el centro de su celda y el dibujo se baja para
// que los pinches tapen su celda. El lanzallamas va centrado en su celda, que es la que quema: la
// boquilla queda contra la pared y el fuego sobre la celda.
// Los pies quedan CHARACTER_HALF_HEIGHT de celda abajo del centro: hasta el borde de la celda
// faltan 4 píxeles.
// Cada trampa ocupa una celda (los cuadros de 16x16); el dibujo del lanzallamas, dos de largo.
inline constexpr int LADO_TRAMPA = 16;
inline constexpr int BAJADA_TRAMPA = 4;
inline const std::chrono::duration<float> CUADRO_TRAMPA(0.08f);
inline const std::map<Pose, HojaDePose> HOJAS_PINCHES = hojas_de(
        {RUTA_PINCHES_QUIETOS, formato_tira(LADO_TRAMPA, LADO_TRAMPA, 1, CUADRO_TRAMPA), 0,
         BAJADA_TRAMPA},
        {{Pose::Atacando,
          {RUTA_PINCHES_ACTIVOS, formato_tira(LADO_TRAMPA, LADO_TRAMPA, 3, CUADRO_TRAMPA, false), 0,
           BAJADA_TRAMPA}}});
inline const std::map<Pose, HojaDePose> HOJAS_LANZALLAMAS_ABAJO = hojas_de(
        {RUTA_LANZALLAMAS_ABAJO_QUIETO,
         formato_tira(LADO_TRAMPA, 2 * LADO_TRAMPA, 1, CUADRO_TRAMPA), 0, BAJADA_TRAMPA},
        {{Pose::Atacando,
          {RUTA_LANZALLAMAS_ABAJO_ACTIVO,
           formato_tira(LADO_TRAMPA, 2 * LADO_TRAMPA, 2, CUADRO_TRAMPA), 0, BAJADA_TRAMPA}}});
inline const std::map<Pose, HojaDePose> HOJAS_LANZALLAMAS_DERECHA = hojas_de(
        {RUTA_LANZALLAMAS_DERECHA_QUIETO,
         formato_tira(2 * LADO_TRAMPA, LADO_TRAMPA, 1, CUADRO_TRAMPA), 0, BAJADA_TRAMPA},
        {{Pose::Atacando,
          {RUTA_LANZALLAMAS_DERECHA_ACTIVO,
           formato_tira(2 * LADO_TRAMPA, LADO_TRAMPA, 2, CUADRO_TRAMPA), 0, BAJADA_TRAMPA}}});
inline const std::map<Pose, HojaDePose> HOJAS_LANZALLAMAS_IZQUIERDA = hojas_de(
        {RUTA_LANZALLAMAS_IZQUIERDA_QUIETO,
         formato_tira(2 * LADO_TRAMPA, LADO_TRAMPA, 1, CUADRO_TRAMPA), 0, BAJADA_TRAMPA},
        {{Pose::Atacando,
          {RUTA_LANZALLAMAS_IZQUIERDA_ACTIVO,
           formato_tira(2 * LADO_TRAMPA, LADO_TRAMPA, 2, CUADRO_TRAMPA), 0, BAJADA_TRAMPA}}});
// Las que se dibujan como parte del piso: debajo de quien esté encima
inline const std::set<EntityType> EN_EL_PISO = {EntityType::Spikes};
inline const std::map<EntityType, std::map<Pose, HojaDePose>> HOJAS_POR_POSE = {
        {EntityType::Skeleton,
         {{Pose::Quieto,
           {RUTA_ESQUELETO_QUIETO, formato_tira(LADO_CUADRO_ESQUELETO, LADO_CUADRO_ESQUELETO, 6,
                                                std::chrono::duration<float>(0.15f))}},
          {Pose::Caminando,
           {RUTA_ESQUELETO_CAMINANDO, formato_tira(LADO_CUADRO_ESQUELETO, LADO_CUADRO_ESQUELETO, 10,
                                                   std::chrono::duration<float>(0.08f))}},
          {Pose::Atacando,
           {RUTA_ESQUELETO_ATACANDO, formato_tira(LADO_CUADRO_ESQUELETO, LADO_CUADRO_ESQUELETO, 9,
                                                  std::chrono::duration<float>(0.6f / 9))}},
          {Pose::Herido,
           {RUTA_ESQUELETO_HERIDO, formato_tira(LADO_CUADRO_ESQUELETO, LADO_CUADRO_ESQUELETO, 5,
                                                std::chrono::duration<float>(0.4f / 5))}},
          {Pose::Muerto,
           {RUTA_ESQUELETO_MUERTO, formato_tira(LADO_CUADRO_ESQUELETO, LADO_CUADRO_ESQUELETO, 17,
                                                std::chrono::duration<float>(0.08f), false)}}}},
        {EntityType::Vampire, HOJAS_VAMPIRO},
        {EntityType::Vampiress, HOJAS_VAMPIRO},
        {EntityType::Orc,
         {{Pose::Quieto,
           {RUTA_ORCO_QUIETO, formato_tira(ANCHO_CUADRO_ORCO, ALTO_CUADRO_ORCO, 6,
                                           std::chrono::duration<float>(0.15f))}},
          {Pose::Caminando,
           {RUTA_ORCO_CAMINANDO, formato_tira(ANCHO_CUADRO_ORCO, ALTO_CUADRO_ORCO, 8,
                                              std::chrono::duration<float>(0.1f))}},
          {Pose::Atacando,
           {RUTA_ORCO_ATACANDO, formato_tira(ANCHO_CUADRO_ORCO, ALTO_CUADRO_ORCO, 6,
                                             std::chrono::duration<float>(0.6f / 6))}},
          {Pose::Herido,
           {RUTA_ORCO_HERIDO, formato_tira(ANCHO_CUADRO_ORCO, ALTO_CUADRO_ORCO, 4,
                                           std::chrono::duration<float>(0.4f / 4))}},
          {Pose::Muerto,
           {RUTA_ORCO_MUERTO, formato_tira(ANCHO_CUADRO_ORCO, ALTO_CUADRO_ORCO, 4,
                                           std::chrono::duration<float>(0.1f), false)}}}},
        {EntityType::Skull, HOJAS_CALAVERA},
        {EntityType::GreenSkull, HOJAS_CALAVERA},
        {EntityType::Flame, HOJAS_LLAMA},
        {EntityType::GreenFlame, HOJAS_LLAMA},
        {EntityType::Spikes, HOJAS_PINCHES},
        {EntityType::FlamethrowerDown, HOJAS_LANZALLAMAS_ABAJO},
        {EntityType::FlamethrowerRight, HOJAS_LANZALLAMAS_DERECHA},
        {EntityType::FlamethrowerLeft, HOJAS_LANZALLAMAS_IZQUIERDA},
};
// Las que se dibujan teñidas (las demás, sin tinte): el vampiro de negro, la vampira de rojo y
// la calavera verde y sus llamas de verde
inline const std::map<EntityType, SDL_Color> TINTES = {
        {EntityType::Vampire, {90, 90, 110, 255}},
        {EntityType::Vampiress, {255, 110, 110, 255}},
        {EntityType::GreenSkull, {110, 255, 140, 255}},
        {EntityType::GreenFlame, {110, 255, 140, 255}},
};

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
inline const std::map<SoilType, stardew::maps::TileType> TILES_SUELO = {
        {SoilType::Dirt, stardew::maps::TileType::Dirt},
        {SoilType::Tilled, stardew::maps::TileType::TilledSoil},
};

inline constexpr int TAMANO_CUADRO_CULTIVO = 16;
inline const std::map<CropType, int> FILAS_CULTIVOS = {
        {CropType::Strawberry, 1},
        {CropType::Scallion, 3},
        {CropType::Potato, 5},
        {CropType::Pumpkin, 7},
};

inline constexpr int COLUMNA_SEMILLAS = 7;
inline const std::map<ItemQuality, int> COLUMNAS_CALIDAD = {
        {ItemQuality::Base, 8},        {ItemQuality::Good, 9},     {ItemQuality::Excellent, 10},
        {ItemQuality::Incredible, 11}, {ItemQuality::Perfect, 12},
};
inline const std::map<ItemType, CropType> SEMILLAS = {
        {ItemType::StrawberrySeeds, CropType::Strawberry},
        {ItemType::ScallionSeeds, CropType::Scallion},
        {ItemType::PotatoSeeds, CropType::Potato},
        {ItemType::PumpkinSeeds, CropType::Pumpkin},
};
inline const std::map<ItemType, CropType> PRODUCTOS_CULTIVO = {
        {ItemType::Strawberry, CropType::Strawberry},
        {ItemType::Scallion, CropType::Scallion},
        {ItemType::Potato, CropType::Potato},
        {ItemType::Pumpkin, CropType::Pumpkin},
};

inline const std::map<ItemType, SDL2pp::Rect> VETAS_MINERALES = {
        {ItemType::Silver, SDL2pp::Rect(72, 11, 18, 16)},
        {ItemType::Coal, SDL2pp::Rect(72, 43, 18, 16)},
        {ItemType::Lapis, SDL2pp::Rect(72, 75, 18, 16)},
        {ItemType::GoldOre, SDL2pp::Rect(72, 107, 18, 16)},
};

inline const std::map<ItemType, SDL2pp::Rect> ICONOS_MINERALES = {
        {ItemType::Silver, SDL2pp::Rect(100, 17, 10, 10)},
        {ItemType::Coal, SDL2pp::Rect(100, 49, 10, 10)},
        {ItemType::Lapis, SDL2pp::Rect(100, 81, 10, 10)},
        {ItemType::GoldOre, SDL2pp::Rect(100, 113, 10, 10)},
};


inline const std::map<ItemType, Sonido> SONIDOS_AL_RECIBIR = {
        {ItemType::Milk, Sonido::Ordenar},        {ItemType::Egg, Sonido::Recoger},
        {ItemType::Strawberry, Sonido::Cosechar}, {ItemType::Scallion, Sonido::Cosechar},
        {ItemType::Potato, Sonido::Cosechar},     {ItemType::Pumpkin, Sonido::Cosechar},
};

inline const std::map<ChatKind, SDL_Color> COLORES_CHAT = {
        {ChatKind::Public, {255, 255, 255, 255}},
        {ChatKind::Private, {255, 150, 200, 255}},
        {ChatKind::System, {170, 170, 170, 255}},
        {ChatKind::Merchant, {140, 220, 140, 255}},
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
// input_madera_panel.png: el marco de las barras de vida y energia, que se estira sin tocar el
// borde. La de vida va a la izquierda de la de energia
inline constexpr int BORDE_MARCO = 9;
inline constexpr int ANCHO_BARRA_HUD = 26;
inline constexpr int ALTO_BARRA_HUD = 80;
inline constexpr int PORCENTAJE_BARRA_MEDIA = 50;
inline constexpr int PORCENTAJE_BARRA_BAJA = 20;
inline constexpr int SEPARACION_BARRAS = 4;
inline const char* const ETIQUETA_ENERGIA = "E";
inline const char* const ETIQUETA_VIDA = "V";
inline constexpr SDL_Color COLOR_TEXTO_HUD = {255, 240, 210, 255};
inline constexpr SDL_Color COLOR_SOMBRA_HUD = {43, 28, 35, 255};
inline constexpr SDL_Color COLOR_BARRA_ALTA = {120, 200, 60, 255};
inline constexpr SDL_Color COLOR_BARRA_MEDIA = {230, 190, 50, 255};
inline constexpr SDL_Color COLOR_BARRA_BAJA = {200, 60, 50, 255};

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

inline constexpr int TAMANO_FUENTE_TITULO = 32;
inline const char* const TITULO_PAUSA = "Pausa";
inline const std::vector<BotonPausa> BOTONES_PAUSA = {{OpcionPausa::Reanudar, "Reanudar", true},
                                                      {OpcionPausa::Invitar, "Invitar", true},
                                                      {OpcionPausa::Salir, "Salir", true}};
inline constexpr int SEPARACION_BOTONES = 12;
inline constexpr int SEPARACION_TITULO_PAUSA = 40;
inline constexpr Uint8 BRILLO_SELECCIONADO = 255;
inline constexpr Uint8 BRILLO_NO_SELECCIONADO = 170;
inline constexpr SDL_Color COLOR_FONDO_PAUSA = {30, 18, 10, 190};
inline constexpr SDL_Color COLOR_TITULO_PAUSA = {255, 235, 173, 255};
inline constexpr SDL_Color COLOR_TEXTO_BOTON = {255, 243, 205, 255};
inline constexpr SDL_Color COLOR_TEXTO_DESHABILITADO = {90, 58, 34, 255};

// La pantalla de fin de día: mismo fondo y botón que la pausa
inline const char* const TITULO_FIN_DEL_DIA = "Terminó el día ";
inline const char* const TEXTO_DURMIO = "Dormiste en tu cama";
inline const char* const TEXTO_NO_DURMIO =
        "No llegaste a tu cama: mañana despertás con la mitad de vida y energía";
inline const char* const TEXTO_OK = "Ok";
inline const char* const TEXTO_ESPERANDO = "Esperando a los demás";
// Entre el título, el texto y el botón
inline constexpr int SEPARACION_TEXTOS_FIN_DEL_DIA = 40;

// El mapa general (M): al abrirlo entra entero en la ventana, dejando este margen a cada lado.
// La rueda acerca o aleja de a ZOOM_PASO, entre el mapa entero y ZOOM_MAXIMO veces eso.
inline constexpr float MARGEN_MAPA_GENERAL = 0.05f;
inline constexpr float ZOOM_PASO = 1.25f;
inline constexpr float ZOOM_MAXIMO = 8.0f;
// Cada jugador es un punto blanco con borde negro; el propio, más grande. El radio va en casillas
// del mapa, con un mínimo en píxeles para que se vea con el mapa entero.
inline constexpr SDL_Color COLOR_PUNTO_JUGADOR = {255, 255, 255, 255};
inline constexpr SDL_Color COLOR_BORDE_PUNTO = {0, 0, 0, 255};
inline constexpr float RADIO_PUNTO = 0.5f;
inline constexpr float RADIO_PUNTO_PROPIO = 0.9f;
inline constexpr int RADIO_MINIMO_PUNTO = 3;
inline constexpr int RADIO_MINIMO_PUNTO_PROPIO = 5;
// El minimapa del HUD, arriba a la izquierda: el área donde está el jugador, en el marco de madera
// del HUD. El área se agranda hasta este tamaño sin deformarse; los tamaños y los radios van en
// píxeles del HUD (se multiplican por su escala).
inline constexpr int ANCHO_MAXIMO_MINIMAPA = 110;
inline constexpr int ALTO_MAXIMO_MINIMAPA = 75;
inline constexpr int RADIO_PUNTO_MINIMAPA = 1;
inline constexpr int RADIO_PUNTO_PROPIO_MINIMAPA = 2;

#endif
