#ifndef RUTAS_H
#define RUTAS_H

#include <map>

#include "client/audio/sonido.h"
#include "client/gui/inventario/objeto.h"

inline const char* const CARPETA_ASSETS = "Assets/";

inline const char* const RUTA_HOJA_VACA = "Assets/Farm/Animals/Female Cow Brown.png";
inline const char* const RUTA_HOJA_GALLINA = "Assets/Farm/Animals/Chicken Red.png";

inline const std::map<Objeto, const char*> RUTAS_ICONOS = {
        {Objeto::Pico, "Assets/Misc/pickaxe.png"},
        {Objeto::Pala, "Assets/Misc/shovel.png"},
        {Objeto::Espada, "Assets/Misc/sword.png"},
        {Objeto::Cama, "Assets/Farm/Environment/Tilesets_Furniture.png"},
        {Objeto::Leche, "Assets/Farm/Animals/milk.png"},
        {Objeto::Huevo, "Assets/Farm/Animals/egg.png"},
};

inline const std::map<Sonido, const char*> RUTAS_SONIDOS = {
        {Sonido::Plantar, "Assets/Sounds/Farming/seed_plant_01.ogg"},
        {Sonido::Regar, "Assets/Sounds/Farming/water_pour_01.ogg"},
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
