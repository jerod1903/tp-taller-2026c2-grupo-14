#pragma once

#include <map>

#include "common/map/MapDefinition.h"

// Que dibujo le toca a cada tile y a cada elemento del mapa. Lo comparten el juego (SDL) y el
// editor (Qt): por eso son solo rutas y rectangulos, sin tipos de ninguna de las dos librerias.
namespace stardew::maps {

// Lado de una casilla, en pixeles de los tilesets (sin escalar)
inline constexpr int TILE_SIZE = 16;

// Rutas de las imagenes, relativas a la carpeta Assets/
inline const char* const SHEET_CAVE = "Caverns/Environment/Dungeon_Tileset.png";
inline const char* const SHEET_OUTDOOR = "Misc/town.png";
inline const char* const SHEET_CABINS = "Farm/Environment/Tilesets_Cabins.png";
inline const char* const SHEET_TOWN_BUILDINGS =
        "Market/World/StradewValley/PelicanTown_Buildings.png";
inline const char* const SHEET_MARNIE = "Market/NPCs/NPC_Marnie.png";
inline const char* const SHEET_CLINT = "Market/NPCs/NPC_Clint.png";
inline const char* const SHEET_PIERRE = "Market/NPCs/NPC_Pierre.png";
inline const char* const SHEET_ROBIN = "Market/NPCs/NPC_Robin.png";
inline const char* const SHEET_ORE = "Caverns/Mining/ore.png";

struct SpriteRect {
    int x;
    int y;
    int width;
    int height;
};

// Casilla (columna, fila) dentro de un tileset
struct TileSprite {
    const char* sheet;
    int column;
    int row;
};

struct ElementSprite {
    const char* sheet;
    SpriteRect rect;
    // Se dibuja con el piso, debajo de los jugadores (por ejemplo, la entrada a la mina)
    bool ground;
};

inline const std::map<TileType, TileSprite> TILE_SPRITES = {
        // Caverna
        {TileType::CaveWall, {SHEET_CAVE, 1, 0}},
        {TileType::CaveFloor, {SHEET_CAVE, 6, 0}},
        // Exterior
        {TileType::Grass, {SHEET_OUTDOOR, 2, 2}},
        {TileType::Dirt, {SHEET_OUTDOOR, 9, 8}},
        {TileType::Path, {SHEET_OUTDOOR, 10, 7}},
        {TileType::TilledSoil, {SHEET_OUTDOOR, 12, 7}},
        {TileType::Water, {SHEET_OUTDOOR, 12, 19}},
        {TileType::Sand, {SHEET_OUTDOOR, 8, 1}},
        {TileType::Stone, {SHEET_OUTDOOR, 13, 16}},
        {TileType::WoodFloor, {SHEET_OUTDOOR, 2, 9}},
};

// Los elementos que no estan aca no se ven en el juego: el spawn y los portones de los pasajes
inline const std::map<ElementType, ElementSprite> ELEMENT_SPRITES = {
        {ElementType::Cabin, {SHEET_CABINS, {0, 0, 80, 112}, false}},
        {ElementType::Market, {SHEET_TOWN_BUILDINGS, {80, 176, 160, 144}, false}},  // el de Pierre
        {ElementType::CommunityCenter, {SHEET_TOWN_BUILDINGS, {0, 640, 192, 160}, false}},
        // El porton doble del tileset de la caverna; se pisa para entrar, va con el piso
        {ElementType::DungeonEntrance,
         {SHEET_CAVE, {6 * TILE_SIZE, 3 * TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE}, true}},
        // La escalera de mano; se pisa para bajar
        {ElementType::Stairs,
         {SHEET_CAVE, {9 * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE}, true}},
        // La puerta simple del tileset de la caverna; se pisa para salir
        {ElementType::Exit,
         {SHEET_CAVE, {8 * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE}, true}},
        // La piedra chica, sin mineral a la vista: cual sale lo decide el servidor
        {ElementType::OreNode, {SHEET_ORE, {72, 11, 18, 16}, false}},
        // Mirando hacia abajo
        {ElementType::MerchantMarnie, {SHEET_MARNIE, {0, 0, 16, 32}, false}},
        {ElementType::MerchantClint, {SHEET_CLINT, {0, 0, 16, 32}, false}},
        {ElementType::MerchantPierre, {SHEET_PIERRE, {0, 0, 16, 32}, false}},
        {ElementType::MerchantRobin, {SHEET_ROBIN, {0, 0, 16, 32}, false}},
};

}  // namespace stardew::maps
