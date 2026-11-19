#ifndef COLORES_MAPA_H
#define COLORES_MAPA_H

#include <map>

#include "client/mapa-general/grilla_mapa.h"
#include "common/map/MapDefinition.h"

// Los colores del mapa general. Los comparten el juego (SDL) y el menú (Qt): por eso no están en
// client/constants.h, que es del juego.

// Lo que queda entre las áreas
inline constexpr ColorMapa COLOR_FONDO{20, 20, 20};
// Un terreno que no está en la tabla (para que se note)
inline constexpr ColorMapa COLOR_DESCONOCIDO{255, 0, 255};

inline const std::map<stardew::maps::TileType, ColorMapa> COLORES_TERRENO = {
        {stardew::maps::TileType::Grass, {106, 170, 70}},
        {stardew::maps::TileType::Dirt, {150, 110, 70}},
        {stardew::maps::TileType::TilledSoil, {110, 75, 45}},
        {stardew::maps::TileType::Water, {60, 120, 200}},
        {stardew::maps::TileType::Path, {205, 185, 140}},
        {stardew::maps::TileType::Sand, {230, 215, 160}},
        {stardew::maps::TileType::WoodFloor, {160, 110, 60}},
        {stardew::maps::TileType::Stone, {120, 120, 125}},
        {stardew::maps::TileType::CaveFloor, {95, 80, 70}},
        {stardew::maps::TileType::CaveWall, {40, 32, 30}},
};

// Los que se ven en el mapa general; los demás (el lugar de los animales, los muebles de los
// interiores) no se dibujan
inline const std::map<stardew::maps::ElementType, ColorMapa> COLORES_ELEMENTO = {
        {stardew::maps::ElementType::Cabin, {200, 60, 60}},
        {stardew::maps::ElementType::Market, {200, 120, 40}},
        {stardew::maps::ElementType::CommunityCenter, {170, 60, 170}},
        {stardew::maps::ElementType::DungeonEntrance, {0, 0, 0}},
        {stardew::maps::ElementType::Tree, {20, 90, 30}},
        {stardew::maps::ElementType::Bush, {50, 130, 50}},
        {stardew::maps::ElementType::Fence, {120, 80, 40}},
        {stardew::maps::ElementType::Well, {80, 80, 160}},
        {stardew::maps::ElementType::Barrel, {140, 90, 40}},
        {stardew::maps::ElementType::Crate, {180, 140, 80}},
        {stardew::maps::ElementType::Flowers, {240, 120, 200}},
        {stardew::maps::ElementType::Statue, {220, 220, 220}},
        {stardew::maps::ElementType::Lamp, {255, 230, 80}},
        {stardew::maps::ElementType::Torch, {255, 140, 0}},
        {stardew::maps::ElementType::Rock, {170, 170, 170}},
        {stardew::maps::ElementType::Exit, {0, 255, 255}},
        {stardew::maps::ElementType::Stairs, {255, 0, 0}},
        {stardew::maps::ElementType::Door, {255, 255, 255}},
};

#endif
