#pragma once

#include <map>
#include <optional>

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
inline const char* const SHEET_TREE = "Misc/tree2/tree1.png";
inline const char* const SHEET_BUSHES = "Misc/tree_01_16x16.png";
inline const char* const SHEET_FARMHOUSE = "Farm/Environment/Tilesets_Farmhouse.png";
inline const char* const SHEET_CHEST = "Misc/chest.png";
inline const char* const SHEET_TOWN_INTERIORS =
        "Market/World/StradewValley/Tilesets_Town_Interiors.png";
inline const char* const SHEET_FENCE = "Farm/Environment/Fence's copiar.png";
inline const char* const SHEET_WELL = "Misc/spr_deco_well.png";
inline const char* const SHEET_STATUE = "Misc/statue.png";
inline const char* const SHEET_BARREL = "Misc/spr_deco_barrel_closed.png";
inline const char* const SHEET_CRATE = "Misc/spr_deco_crate_01.png";
inline const char* const SHEET_FLOWERS = "Misc/spr_deco_flowers_house_02.png";
inline const char* const SHEET_FARM_OBJECTS = "Farm/Environment/objects_01_16x16.png";
inline const char* const SHEET_BASKET = "Misc/basket.png";

struct SpriteRect {
    int x;
    int y;
    int width;
    int height;
};

// Casilla (columna, fila) dentro de un tileset. Algunas piezas no caen justo en la grilla (se
// corren offsetX pixeles), miden menos de alto (el resto queda negro), tienen partes
// transparentes (debajo se dibuja el tile `under`) o se usan dadas vuelta de izquierda a derecha
struct TileSprite {
    const char* sheet;
    int column;
    int row;
    int offsetX = 0;
    int height = TILE_SIZE;
    std::optional<TileType> under = std::nullopt;
    bool flipped = false;
};

struct ElementSprite {
    const char* sheet;
    SpriteRect rect;
    // Se dibuja con el piso, debajo de los jugadores (por ejemplo, la entrada a la mina)
    bool ground;
    bool flipped = false;  // dado vuelta de izquierda a derecha
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
        // Interiores, del tileset de la casa
        {TileType::HouseFloor, {SHEET_FARMHOUSE, 1, 8}},
        // En el juego se ve como el piso: lo que se dibuja es la cama, cuando aparece
        {TileType::Sleepable, {SHEET_FARMHOUSE, 1, 8}},
        {TileType::WallTop, {SHEET_FARMHOUSE, 2, 0}},
        {TileType::WallUpper, {SHEET_FARMHOUSE, 2, 1}},
        {TileType::Wall, {SHEET_FARMHOUSE, 2, 2}},
        {TileType::WallBase, {SHEET_FARMHOUSE, 2, 3}},
        {TileType::WallLeft, {SHEET_FARMHOUSE, 0, 2}},
        {TileType::WallRight, {SHEET_FARMHOUSE, 11, 2}},
        // El tramo recto del zocalo de abajo, entre dos casillas del tileset, sobre el piso
        {TileType::WallBottom, {SHEET_FARMHOUSE, 0, 11, 8, TILE_SIZE, TileType::HouseFloor}},
        // Las molduras de los costados bajan hasta la linea del zocalo
        {TileType::WallBottomLeft, {SHEET_FARMHOUSE, 0, 2, 0, 12}},
        {TileType::WallBottomRight, {SHEET_FARMHOUSE, 11, 2, 0, 12}},
        // Donde el zocalo dobla hacia abajo para seguir por el pasillo de entrada
        {TileType::CorridorLeft, {SHEET_FARMHOUSE, 1, 11, 0, TILE_SIZE, TileType::HouseFloor}},
        {TileType::CorridorRight,
         {SHEET_FARMHOUSE, 1, 11, 0, TILE_SIZE, TileType::HouseFloor, true}},
        {TileType::WallDivider, {SHEET_FARMHOUSE, 10, 7}},  // la pared lateral, de tablas
        {TileType::Void, {SHEET_FARMHOUSE, 0, 0}},
};

// Los elementos que no estan aca no se ven en el juego: donde arrancan los animales y los portones
// de los pasajes
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
        // La piedra chica, sin mineral a la vista. Es un obstaculo: los minerales los pone el
        // servidor, no el mapa
        {ElementType::Rock, {SHEET_ORE, {72, 11, 18, 16}, false}},
        // El pino, hasta la sombra del tronco: la copa sobresale hacia arriba
        {ElementType::Tree, {SHEET_TREE, {0, 0, 28, 40}, false}},
        // El arbusto chico
        {ElementType::Bush, {SHEET_BUSHES, {1, 32, 30, 32}, false}},
        // Decoracion. El tramo de cerco con el poste al medio, que empalma con el de al lado
        {ElementType::Fence, {SHEET_FENCE, {16, 32, 16, 16}, false}},
        {ElementType::Well, {SHEET_WELL, {0, 0, 20, 25}, false}},
        // La del rey, la de la izquierda de la hoja
        {ElementType::Statue, {SHEET_STATUE, {6, 1, 32, 55}, false}},
        {ElementType::Barrel, {SHEET_BARREL, {0, 0, 13, 16}, false}},
        {ElementType::Crate, {SHEET_CRATE, {0, 0, 16, 21}, false}},
        {ElementType::Flowers, {SHEET_FLOWERS, {0, 0, 12, 19}, false}},
        // El farol rojo
        {ElementType::Lamp, {SHEET_FARM_OBJECTS, {48, 80, 16, 32}, false}},
        // La antorcha encendida del tileset de la caverna
        {ElementType::Torch, {SHEET_CAVE, {0, 144, 16, 16}, false}},
        // Mirando hacia abajo
        {ElementType::MerchantMarnie, {SHEET_MARNIE, {0, 0, 16, 32}, false}},
        {ElementType::MerchantClint, {SHEET_CLINT, {0, 0, 16, 32}, false}},
        {ElementType::MerchantPierre, {SHEET_PIERRE, {0, 0, 16, 32}, false}},
        {ElementType::MerchantRobin, {SHEET_ROBIN, {0, 0, 16, 32}, false}},
        // Interior de la cabaña, del tileset de la casa (el cofre no esta ahi). La puerta de salida
        // no se dibuja: es el final del pasillo de entrada
        // La cama simple: cabecera con almohada y, abajo, la frazada con el pie
        {ElementType::Bed, {SHEET_FARMHOUSE, {128, 112, 32, 32}, false}},
        {ElementType::BedBlanket, {SHEET_FARMHOUSE, {128, 144, 32, 32}, false}},
        // Justo los ladrillos: al costado el tileset sigue con la pared en diagonal
        {ElementType::Fireplace, {SHEET_FARMHOUSE, {131, 0, 32, 80}, false}},
        {ElementType::Window, {SHEET_FARMHOUSE, {48, 16, 16, 32}, false}},
        {ElementType::Shelf, {SHEET_FARMHOUSE, {80, 16, 32, 32}, false}},
        {ElementType::Tv, {SHEET_FARMHOUSE, {96, 192, 32, 48}, false}},
        {ElementType::Kitchen, {SHEET_FARMHOUSE, {16, 192, 80, 48}, false}},
        // El tileset trae la mitad derecha de la alfombra: la izquierda es la misma espejada
        {ElementType::Rug, {SHEET_FARMHOUSE, {96, 128, 32, 32}, true}},
        {ElementType::RugLeft, {SHEET_FARMHOUSE, {96, 128, 32, 32}, true, true}},
        {ElementType::Plant, {SHEET_FARMHOUSE, {64, 304, 16, 48}, false}},
        {ElementType::Chest, {SHEET_CHEST, {8, 0, 16, 16}, false}},
        // Interior del mercado: el mostrador con la caja y la vitrina, y el estante con mercaderia
        {ElementType::MarketCounter, {SHEET_TOWN_INTERIORS, {304, 480, 48, 32}, false}},
        {ElementType::MarketShelf, {SHEET_TOWN_INTERIORS, {416, 464, 96, 48}, false}},
        // Interior del centro comunitario: un objeto por set, donde se entrega
        {ElementType::CropsBundle, {SHEET_FARM_OBJECTS, {48, 112, 16, 15}, false}},  // cartel
        {ElementType::MetalsBundle, {SHEET_ORE, {4, 98, 26, 25}, false}},  // piedra con oro
        {ElementType::AnimalBundle, {SHEET_BASKET, {0, 0, 12, 12}, false}},
        {ElementType::MoneyBundle, {SHEET_FARM_OBJECTS, {2, 50, 12, 13}, false}},  // bolsa
};

}  // namespace stardew::maps
