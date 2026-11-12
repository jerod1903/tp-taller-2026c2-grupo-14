#ifndef RUTAS_H
#define RUTAS_H

#include <map>

#include "client/audio/sonido.h"

// Rutas de los assets, relativas a la raiz del repo (el juego se corre desde ahi)

// mapa.cpp y constructor_capas.cpp: las imagenes del mapa y del granjero estan en
// common/map/MapSprites.h y common/character/CharacterSheets.h, con rutas relativas a esta carpeta
inline const char* const CARPETA_ASSETS = "Assets/";

// interfaz.cpp: los animales de la granja
inline const char* const RUTA_HOJA_VACA = "Assets/Farm/Animals/Female Cow Brown.png";
inline const char* const RUTA_HOJA_GALLINA = "Assets/Farm/Animals/Chicken Red.png";

// reproductor_sonidos.cpp
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
