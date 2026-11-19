#ifndef RUTAS_H
#define RUTAS_H

#include <map>

#include "client/audio/sonido.h"
#include "common/game_dto.h"

// TALLER_ASSETS_DIR: la carpeta Assets del repo o la instalada (ver CMakeLists.txt)
inline const char* const CARPETA_ASSETS = TALLER_ASSETS_DIR "/";

inline const char* const RUTA_FUENTE = TALLER_ASSETS_DIR "/Fonts/PixelifySans.ttf";

inline const char* const RUTA_GLOBO_ESCRIBIENDO = TALLER_ASSETS_DIR "/Misc/expression_chat.png";
inline const char* const RUTA_ICONO_LECHE = TALLER_ASSETS_DIR "/Farm/Animals/milk.png";
inline const char* const RUTA_ICONO_REGADERA = TALLER_ASSETS_DIR "/Misc/watering_can.png";

inline const char* const RUTA_CARTELES_HUD = TALLER_ASSETS_DIR "/Menu/botones/botones_madera_enlazados.png";
inline const char* const RUTA_MARCO_HUD = TALLER_ASSETS_DIR "/Menu/botones/input_madera_panel.png";
inline const char* const RUTA_ICONO_RELOJ = TALLER_ASSETS_DIR "/Misc/stopwatch.png";
inline const char* const RUTA_ICONO_ORO = TALLER_ASSETS_DIR "/Caverns/items and trap_animation/coin/coin_1.png";

inline const char* const RUTA_HOJA_VACA = TALLER_ASSETS_DIR "/Farm/Animals/Female Cow Brown.png";
inline const char* const RUTA_HOJA_GALLINA = TALLER_ASSETS_DIR "/Farm/Animals/Chicken Red.png";
inline const char* const RUTA_HOJA_CULTIVOS = TALLER_ASSETS_DIR "/Farm/Crops/Spring Crops.png";

inline const char* const RUTA_ESQUELETO_QUIETO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_idle.png";
inline const char* const RUTA_ESQUELETO_CAMINANDO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_movement.png";
inline const char* const RUTA_ESQUELETO_ATACANDO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_attack.png";
inline const char* const RUTA_ESQUELETO_HERIDO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_take_damage.png";
inline const char* const RUTA_ESQUELETO_MUERTO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_death.png";

inline const char* const RUTA_VAMPIRO_QUIETO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Vampire/enemies-vampire_idle.png";
inline const char* const RUTA_VAMPIRO_CAMINANDO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Vampire/enemies-vampire_movement.png";
inline const char* const RUTA_VAMPIRO_ATACANDO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Vampire/enemies-vampire_attack.png";
inline const char* const RUTA_VAMPIRO_HERIDO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Vampire/enemies-vampire_take_damage.png";
inline const char* const RUTA_VAMPIRO_MUERTO =
        TALLER_ASSETS_DIR "/Caverns/Enemies/Vampire/enemies-vampire_death.png";

inline const char* const RUTA_ORCO_QUIETO = TALLER_ASSETS_DIR "/Caverns/Enemies/Orcs/Orc_Idle.png";
inline const char* const RUTA_ORCO_CAMINANDO = TALLER_ASSETS_DIR "/Caverns/Enemies/Orcs/Orc_Walk.png";
inline const char* const RUTA_ORCO_ATACANDO = TALLER_ASSETS_DIR "/Caverns/Enemies/Orcs/Orc_Attack01.png";
inline const char* const RUTA_ORCO_HERIDO = TALLER_ASSETS_DIR "/Caverns/Enemies/Orcs/Orc_Hurt.png";
inline const char* const RUTA_ORCO_MUERTO = TALLER_ASSETS_DIR "/Caverns/Enemies/Orcs/Orc_Death.png";

inline const char* const RUTA_CALAVERA = TALLER_ASSETS_DIR "/Caverns/Enemies/Skull/skull_v2.png";
inline const char* const RUTA_LLAMA = TALLER_ASSETS_DIR "/Caverns/Enemies/Skull/fire/flame.png";

inline const char* const RUTA_PINCHES_QUIETOS =
        TALLER_ASSETS_DIR "/Caverns/items and trap_animation/peaks/spikes_idle.png";
inline const char* const RUTA_PINCHES_ACTIVOS =
        TALLER_ASSETS_DIR "/Caverns/items and trap_animation/peaks/spikes_active.png";
inline const char* const RUTA_LANZALLAMAS_ABAJO_QUIETO =
        TALLER_ASSETS_DIR "/Caverns/items and trap_animation/flamethrower/down_idle.png";
inline const char* const RUTA_LANZALLAMAS_ABAJO_ACTIVO =
        TALLER_ASSETS_DIR "/Caverns/items and trap_animation/flamethrower/down_active.png";
inline const char* const RUTA_LANZALLAMAS_DERECHA_QUIETO =
        TALLER_ASSETS_DIR "/Caverns/items and trap_animation/flamethrower/right_idle.png";
inline const char* const RUTA_LANZALLAMAS_DERECHA_ACTIVO =
        TALLER_ASSETS_DIR "/Caverns/items and trap_animation/flamethrower/right_active.png";
inline const char* const RUTA_LANZALLAMAS_IZQUIERDA_QUIETO =
        TALLER_ASSETS_DIR "/Caverns/items and trap_animation/flamethrower/left_idle.png";
inline const char* const RUTA_LANZALLAMAS_IZQUIERDA_ACTIVO =
        TALLER_ASSETS_DIR "/Caverns/items and trap_animation/flamethrower/left_active.png";

inline const char* const RUTA_BOTON_MENU = TALLER_ASSETS_DIR "/Menu/botones/boton_320.png";
inline const char* const RUTA_BOTON_DESHABILITADO =
        TALLER_ASSETS_DIR "/Menu/botones/boton_deshabilitado_320.png";

inline const std::map<ItemType, const char*> RUTAS_ICONOS = {
        {ItemType::Pickaxe, TALLER_ASSETS_DIR "/Misc/pickaxe.png"},
        {ItemType::WateringCan, TALLER_ASSETS_DIR "/Misc/watering_can.png"},
        {ItemType::Shovel, TALLER_ASSETS_DIR "/Misc/shovel.png"},
        {ItemType::Sword, TALLER_ASSETS_DIR "/Misc/sword.png"},
        {ItemType::Milk, TALLER_ASSETS_DIR "/Farm/Animals/milk.png"},
        {ItemType::Egg, TALLER_ASSETS_DIR "/Farm/Animals/egg.png"},
};

inline const std::map<Sonido, const char*> RUTAS_SONIDOS = {
        {Sonido::Plantar, TALLER_ASSETS_DIR "/Sounds/Farming/seed_plant_01.ogg"},
        {Sonido::Regar, TALLER_ASSETS_DIR "/Sounds/Farming/water_pour_01.ogg"},
        {Sonido::Ordenar, TALLER_ASSETS_DIR "/Sounds/Farming/milking_01.ogg"},
        {Sonido::Cosechar, TALLER_ASSETS_DIR "/Sounds/Farming/harvest_pluck_01.ogg"},
        {Sonido::Azada, TALLER_ASSETS_DIR "/Sounds/Farming/hoe_soil_01.ogg"},
        {Sonido::Recoger, TALLER_ASSETS_DIR "/Sounds/Gathering/item_pickup.ogg"},
        {Sonido::Hacha, TALLER_ASSETS_DIR "/Sounds/Gathering/wood_chop_01.ogg"},
        {Sonido::Pico, TALLER_ASSETS_DIR "/Sounds/Gathering/hammer_tap_01.ogg"},
        {Sonido::LanzarCana, TALLER_ASSETS_DIR "/Sounds/Fishing/cast_01.ogg"},
        {Sonido::Boya, TALLER_ASSETS_DIR "/Sounds/Fishing/bobber_plop_01.ogg"},
        {Sonido::AtraparPez, TALLER_ASSETS_DIR "/Sounds/Fishing/jingle_victory.ogg"},
        {Sonido::Paso, TALLER_ASSETS_DIR "/Sounds/Footsteps/footstep_grass_01.ogg"},
        {Sonido::Moneda, TALLER_ASSETS_DIR "/Sounds/Gathering/coin_01.ogg"},
        {Sonido::NuevoDia, TALLER_ASSETS_DIR "/Sounds/DayCycle/morning_01.ogg"},
        {Sonido::Gallina, TALLER_ASSETS_DIR "/Sounds/Animals/hen_cluck_02.ogg"},
};

#endif
