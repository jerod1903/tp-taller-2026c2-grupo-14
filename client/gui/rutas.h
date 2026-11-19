#ifndef RUTAS_H
#define RUTAS_H

#include <map>

#include "client/audio/sonido.h"
#include "common/game_dto.h"

inline const char* const CARPETA_ASSETS = "Assets/";

inline const char* const RUTA_FUENTE = "Assets/Fonts/PixelifySans.ttf";

inline const char* const RUTA_GLOBO_ESCRIBIENDO = "Assets/Misc/expression_chat.png";
inline const char* const RUTA_ICONO_LECHE = "Assets/Farm/Animals/milk.png";
inline const char* const RUTA_ICONO_REGADERA = "Assets/Misc/watering_can.png";

inline const char* const RUTA_CARTELES_HUD = "Assets/Menu/botones/botones_madera_enlazados.png";
inline const char* const RUTA_MARCO_HUD = "Assets/Menu/botones/input_madera_panel.png";
inline const char* const RUTA_ICONO_RELOJ = "Assets/Misc/stopwatch.png";
inline const char* const RUTA_ICONO_ORO = "Assets/Caverns/items and trap_animation/coin/coin_1.png";

inline const char* const RUTA_HOJA_VACA = "Assets/Farm/Animals/Female Cow Brown.png";
inline const char* const RUTA_HOJA_GALLINA = "Assets/Farm/Animals/Chicken Red.png";
inline const char* const RUTA_HOJA_CULTIVOS = "Assets/Farm/Crops/Spring Crops.png";

inline const char* const RUTA_ESQUELETO_QUIETO =
        "Assets/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_idle.png";
inline const char* const RUTA_ESQUELETO_CAMINANDO =
        "Assets/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_movement.png";
inline const char* const RUTA_ESQUELETO_ATACANDO =
        "Assets/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_attack.png";
inline const char* const RUTA_ESQUELETO_HERIDO =
        "Assets/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_take_damage.png";
inline const char* const RUTA_ESQUELETO_MUERTO =
        "Assets/Caverns/Enemies/Skeleton/V1/enemies-skeleton1_death.png";

inline const char* const RUTA_VAMPIRO_QUIETO =
        "Assets/Caverns/Enemies/Vampire/enemies-vampire_idle.png";
inline const char* const RUTA_VAMPIRO_CAMINANDO =
        "Assets/Caverns/Enemies/Vampire/enemies-vampire_movement.png";
inline const char* const RUTA_VAMPIRO_ATACANDO =
        "Assets/Caverns/Enemies/Vampire/enemies-vampire_attack.png";
inline const char* const RUTA_VAMPIRO_HERIDO =
        "Assets/Caverns/Enemies/Vampire/enemies-vampire_take_damage.png";
inline const char* const RUTA_VAMPIRO_MUERTO =
        "Assets/Caverns/Enemies/Vampire/enemies-vampire_death.png";

inline const char* const RUTA_ORCO_QUIETO = "Assets/Caverns/Enemies/Orcs/Orc_Idle.png";
inline const char* const RUTA_ORCO_CAMINANDO = "Assets/Caverns/Enemies/Orcs/Orc_Walk.png";
inline const char* const RUTA_ORCO_ATACANDO = "Assets/Caverns/Enemies/Orcs/Orc_Attack01.png";
inline const char* const RUTA_ORCO_HERIDO = "Assets/Caverns/Enemies/Orcs/Orc_Hurt.png";
inline const char* const RUTA_ORCO_MUERTO = "Assets/Caverns/Enemies/Orcs/Orc_Death.png";

inline const char* const RUTA_BOTON_MENU = "Assets/Menu/botones/boton_320.png";
inline const char* const RUTA_BOTON_DESHABILITADO =
        "Assets/Menu/botones/boton_deshabilitado_320.png";

inline const std::map<ItemType, const char*> RUTAS_ICONOS = {
        {ItemType::Pickaxe, "Assets/Misc/pickaxe.png"},
        {ItemType::WateringCan, "Assets/Misc/watering_can.png"},
        {ItemType::Shovel, "Assets/Misc/shovel.png"},
        {ItemType::Sword, "Assets/Misc/sword.png"},
        {ItemType::Milk, "Assets/Farm/Animals/milk.png"},
        {ItemType::Egg, "Assets/Farm/Animals/egg.png"},
};

inline const std::map<Sonido, const char*> RUTAS_SONIDOS = {
        {Sonido::Plantar, "Assets/Sounds/Farming/seed_plant_01.ogg"},
        {Sonido::Regar, "Assets/Sounds/Farming/water_pour_01.ogg"},
        {Sonido::Ordenar, "Assets/Sounds/Farming/milking_01.ogg"},
        {Sonido::Cosechar, "Assets/Sounds/Farming/harvest_pluck_01.ogg"},
        {Sonido::Azada, "Assets/Sounds/Farming/hoe_soil_01.ogg"},
        {Sonido::Recoger, "Assets/Sounds/Gathering/item_pickup.ogg"},
        {Sonido::Hacha, "Assets/Sounds/Gathering/wood_chop_01.ogg"},
        {Sonido::Pico, "Assets/Sounds/Gathering/hammer_tap_01.ogg"},
        {Sonido::LanzarCana, "Assets/Sounds/Fishing/cast_01.ogg"},
        {Sonido::Boya, "Assets/Sounds/Fishing/bobber_plop_01.ogg"},
        {Sonido::AtraparPez, "Assets/Sounds/Fishing/jingle_victory.ogg"},
        {Sonido::Paso, "Assets/Sounds/Footsteps/footstep_grass_01.ogg"},
        {Sonido::Moneda, "Assets/Sounds/Gathering/coin_01.ogg"},
        {Sonido::NuevoDia, "Assets/Sounds/DayCycle/morning_01.ogg"},
        {Sonido::Gallina, "Assets/Sounds/Animals/hen_cluck_02.ogg"},
};

#endif
