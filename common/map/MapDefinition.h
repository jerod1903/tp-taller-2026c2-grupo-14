#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace stardew::maps {

enum class AreaType { Farm, Village, DungeonLevel, CabinArea, MarketArea, CommunityCenterArea };

enum class ElementType {
    Cabin,
    Bed,
    BedBlanket,  // La cama se divide en dos elementos para dar ilusión de profundidad.
    Market,
    CommunityCenter,
    DungeonEntrance,
    MerchantMarnie,
    MerchantClint,
    MerchantPierre,
    MerchantRobin,
    Rock,
    Door,
    Exit,
    Stairs,
    AnimalSpawn,  // donde arrancan los animales de la granja; el jugador aparece en su cama
    Tree,
    Bush,
    // Decoracion del exterior y de la mina: la pone el editor y choca
    Fence,
    Well,
    Statue,
    Barrel,
    Crate,
    Flowers,
    Lamp,
    Torch,
    // Muebles del interior de la cabaña
    Fireplace,
    Chest,
    Window,
    Shelf,
    Tv,
    Kitchen,
    Rug,
    RugLeft,  // la alfombra espejada: va a la izquierda de Rug
    Plant,
    HouseDoor,  // la puerta de salida de un interior: se ve y se pisa para salir
    // Muebles del interior del mercado
    MarketCounter,
    MarketShelf,
};

enum class ElementLayer {
    Structure,
    Object,
    Npc,
};

enum class TileType {
    Grass,
    Dirt,
    TilledSoil,
    Water,
    Path,
    Stone,
    CaveFloor,
    CaveWall,
    WoodFloor,
    Sand,
    Sleepable,  // donde va la cama de un jugador: se camina hasta que aparece la cama.
    HouseFloor,
    WallTop,
    WallUpper,
    Wall,
    WallBase,
    WallLeft,
    WallRight,
    WallBottom,
    WallBottomLeft,
    WallBottomRight,
    CorridorLeft,
    CorridorRight,
    WallDivider,
    Void,
};

inline std::optional<TileType> tileTypeFromString(std::string_view value) {
    if (value == "grass")
        return TileType::Grass;
    if (value == "dirt")
        return TileType::Dirt;
    if (value == "tilled_soil")
        return TileType::TilledSoil;
    if (value == "water")
        return TileType::Water;
    if (value == "path")
        return TileType::Path;
    if (value == "stone")
        return TileType::Stone;
    if (value == "cave_floor")
        return TileType::CaveFloor;
    if (value == "cave_wall")
        return TileType::CaveWall;
    if (value == "wood_floor")
        return TileType::WoodFloor;
    if (value == "sand")
        return TileType::Sand;
    if (value == "sleepable")
        return TileType::Sleepable;
    if (value == "house_floor")
        return TileType::HouseFloor;
    if (value == "wall_top")
        return TileType::WallTop;
    if (value == "wall_upper")
        return TileType::WallUpper;
    if (value == "wall")
        return TileType::Wall;
    if (value == "wall_base")
        return TileType::WallBase;
    if (value == "wall_left")
        return TileType::WallLeft;
    if (value == "wall_right")
        return TileType::WallRight;
    if (value == "wall_bottom")
        return TileType::WallBottom;
    if (value == "wall_bottom_left")
        return TileType::WallBottomLeft;
    if (value == "wall_bottom_right")
        return TileType::WallBottomRight;
    if (value == "corridor_left")
        return TileType::CorridorLeft;
    if (value == "corridor_right")
        return TileType::CorridorRight;
    if (value == "wall_divider")
        return TileType::WallDivider;
    if (value == "void")
        return TileType::Void;
    return std::nullopt;
}

inline std::string_view toString(TileType type) {
    switch (type) {
        case TileType::Grass:
            return "grass";
        case TileType::Dirt:
            return "dirt";
        case TileType::TilledSoil:
            return "tilled_soil";
        case TileType::Water:
            return "water";
        case TileType::Path:
            return "path";
        case TileType::Stone:
            return "stone";
        case TileType::CaveFloor:
            return "cave_floor";
        case TileType::CaveWall:
            return "cave_wall";
        case TileType::WoodFloor:
            return "wood_floor";
        case TileType::Sand:
            return "sand";
        case TileType::Sleepable:
            return "sleepable";
        case TileType::HouseFloor:
            return "house_floor";
        case TileType::WallTop:
            return "wall_top";
        case TileType::WallUpper:
            return "wall_upper";
        case TileType::Wall:
            return "wall";
        case TileType::WallBase:
            return "wall_base";
        case TileType::WallLeft:
            return "wall_left";
        case TileType::WallRight:
            return "wall_right";
        case TileType::WallBottom:
            return "wall_bottom";
        case TileType::WallBottomLeft:
            return "wall_bottom_left";
        case TileType::WallBottomRight:
            return "wall_bottom_right";
        case TileType::CorridorLeft:
            return "corridor_left";
        case TileType::CorridorRight:
            return "corridor_right";
        case TileType::WallDivider:
            return "wall_divider";
        case TileType::Void:
            return "void";
    }
    return "unknown_tile";
}

// Agua, paredes de cueva, piedra y las paredes de los interiores no se pueden pisar. Lo usan el
// servidor y el validador.
inline bool isWalkableTerrain(TileType type) {
    switch (type) {
        case TileType::Water:
        case TileType::CaveWall:
        case TileType::Stone:
        case TileType::WallTop:
        case TileType::WallUpper:
        case TileType::Wall:
        case TileType::WallBase:
        case TileType::WallLeft:
        case TileType::WallRight:
        case TileType::WallBottom:
        case TileType::WallBottomLeft:
        case TileType::WallBottomRight:
        case TileType::CorridorLeft:
        case TileType::CorridorRight:
        case TileType::WallDivider:
        case TileType::Void:
            return false;
        default:
            return true;
    }
}

inline std::vector<TileType> allTileTypes() {
    return {
            TileType::Grass,        TileType::Dirt,           TileType::TilledSoil,
            TileType::Water,        TileType::Path,           TileType::Stone,
            TileType::CaveFloor,    TileType::CaveWall,       TileType::WoodFloor,
            TileType::Sand,         TileType::Sleepable,      TileType::HouseFloor,
            TileType::WallTop,      TileType::WallUpper,      TileType::Wall,
            TileType::WallBase,     TileType::WallLeft,       TileType::WallRight,
            TileType::WallBottom,   TileType::WallBottomLeft, TileType::WallBottomRight,
            TileType::CorridorLeft, TileType::CorridorRight,  TileType::WallDivider,
            TileType::Void,
    };
}

inline std::optional<AreaType> areaTypeFromString(std::string_view value) {
    if (value == "farm")
        return AreaType::Farm;
    if (value == "village")
        return AreaType::Village;
    if (value == "dungeon_level")
        return AreaType::DungeonLevel;
    if (value == "cabin_area")
        return AreaType::CabinArea;
    if (value == "market_area")
        return AreaType::MarketArea;
    if (value == "community_center_area")
        return AreaType::CommunityCenterArea;
    return std::nullopt;
}

inline std::string_view toString(AreaType type) {
    switch (type) {
        case AreaType::Farm:
            return "farm";
        case AreaType::Village:
            return "village";
        case AreaType::DungeonLevel:
            return "dungeon_level";
        case AreaType::CabinArea:
            return "cabin_area";
        case AreaType::MarketArea:
            return "market_area";
        case AreaType::CommunityCenterArea:
            return "community_center_area";
    }
    return "unknown_area";
}

inline std::optional<ElementType> elementTypeFromString(std::string_view value) {
    if (value == "cabin")
        return ElementType::Cabin;
    if (value == "bed")
        return ElementType::Bed;
    if (value == "bed_blanket")
        return ElementType::BedBlanket;
    if (value == "market")
        return ElementType::Market;
    if (value == "community_center")
        return ElementType::CommunityCenter;
    if (value == "dungeon_entrance")
        return ElementType::DungeonEntrance;
    if (value == "merchant_marnie")
        return ElementType::MerchantMarnie;
    if (value == "merchant_clint")
        return ElementType::MerchantClint;
    if (value == "merchant_pierre")
        return ElementType::MerchantPierre;
    if (value == "merchant_robin")
        return ElementType::MerchantRobin;
    if (value == "rock")
        return ElementType::Rock;
    if (value == "door")
        return ElementType::Door;
    if (value == "exit")
        return ElementType::Exit;
    if (value == "stairs")
        return ElementType::Stairs;
    if (value == "animal_spawn")
        return ElementType::AnimalSpawn;
    if (value == "tree")
        return ElementType::Tree;
    if (value == "bush")
        return ElementType::Bush;
    if (value == "fence")
        return ElementType::Fence;
    if (value == "well")
        return ElementType::Well;
    if (value == "statue")
        return ElementType::Statue;
    if (value == "barrel")
        return ElementType::Barrel;
    if (value == "crate")
        return ElementType::Crate;
    if (value == "flowers")
        return ElementType::Flowers;
    if (value == "lamp")
        return ElementType::Lamp;
    if (value == "torch")
        return ElementType::Torch;
    if (value == "fireplace")
        return ElementType::Fireplace;
    if (value == "chest")
        return ElementType::Chest;
    if (value == "window")
        return ElementType::Window;
    if (value == "shelf")
        return ElementType::Shelf;
    if (value == "tv")
        return ElementType::Tv;
    if (value == "kitchen")
        return ElementType::Kitchen;
    if (value == "rug")
        return ElementType::Rug;
    if (value == "rug_left")
        return ElementType::RugLeft;
    if (value == "plant")
        return ElementType::Plant;
    if (value == "house_door")
        return ElementType::HouseDoor;
    if (value == "market_counter")
        return ElementType::MarketCounter;
    if (value == "market_shelf")
        return ElementType::MarketShelf;
    return std::nullopt;
}

inline std::string_view toString(ElementType type) {
    switch (type) {
        case ElementType::Cabin:
            return "cabin";
        case ElementType::Bed:
            return "bed";
        case ElementType::BedBlanket:
            return "bed_blanket";
        case ElementType::Market:
            return "market";
        case ElementType::CommunityCenter:
            return "community_center";
        case ElementType::DungeonEntrance:
            return "dungeon_entrance";
        case ElementType::MerchantMarnie:
            return "merchant_marnie";
        case ElementType::MerchantClint:
            return "merchant_clint";
        case ElementType::MerchantPierre:
            return "merchant_pierre";
        case ElementType::MerchantRobin:
            return "merchant_robin";
        case ElementType::Rock:
            return "rock";
        case ElementType::Door:
            return "door";
        case ElementType::Exit:
            return "exit";
        case ElementType::Stairs:
            return "stairs";
        case ElementType::AnimalSpawn:
            return "animal_spawn";
        case ElementType::Tree:
            return "tree";
        case ElementType::Bush:
            return "bush";
        case ElementType::Fence:
            return "fence";
        case ElementType::Well:
            return "well";
        case ElementType::Statue:
            return "statue";
        case ElementType::Barrel:
            return "barrel";
        case ElementType::Crate:
            return "crate";
        case ElementType::Flowers:
            return "flowers";
        case ElementType::Lamp:
            return "lamp";
        case ElementType::Torch:
            return "torch";
        case ElementType::Fireplace:
            return "fireplace";
        case ElementType::Chest:
            return "chest";
        case ElementType::Window:
            return "window";
        case ElementType::Shelf:
            return "shelf";
        case ElementType::Tv:
            return "tv";
        case ElementType::Kitchen:
            return "kitchen";
        case ElementType::Rug:
            return "rug";
        case ElementType::RugLeft:
            return "rug_left";
        case ElementType::Plant:
            return "plant";
        case ElementType::HouseDoor:
            return "house_door";
        case ElementType::MarketCounter:
            return "market_counter";
        case ElementType::MarketShelf:
            return "market_shelf";
    }
    return "unknown_element";
}

inline std::vector<ElementType> allElementTypes() {
    return {
            ElementType::Cabin,
            ElementType::Bed,
            ElementType::BedBlanket,
            ElementType::Market,
            ElementType::CommunityCenter,
            ElementType::DungeonEntrance,
            ElementType::MerchantMarnie,
            ElementType::MerchantClint,
            ElementType::MerchantPierre,
            ElementType::MerchantRobin,
            ElementType::Rock,
            ElementType::Door,
            ElementType::Exit,
            ElementType::Stairs,
            ElementType::AnimalSpawn,
            ElementType::Tree,
            ElementType::Bush,
            ElementType::Fence,
            ElementType::Well,
            ElementType::Statue,
            ElementType::Barrel,
            ElementType::Crate,
            ElementType::Flowers,
            ElementType::Lamp,
            ElementType::Torch,
            ElementType::Fireplace,
            ElementType::Chest,
            ElementType::Window,
            ElementType::Shelf,
            ElementType::Tv,
            ElementType::Kitchen,
            ElementType::Rug,
            ElementType::RugLeft,
            ElementType::Plant,
            ElementType::HouseDoor,
            ElementType::MarketCounter,
            ElementType::MarketShelf,
    };
}

inline std::optional<ElementLayer> elementLayerFromString(std::string_view value) {
    if (value == "structure")
        return ElementLayer::Structure;
    if (value == "object")
        return ElementLayer::Object;
    if (value == "npc")
        return ElementLayer::Npc;
    return std::nullopt;
}

inline std::string_view toString(ElementLayer layer) {
    switch (layer) {
        case ElementLayer::Structure:
            return "structure";
        case ElementLayer::Object:
            return "object";
        case ElementLayer::Npc:
            return "npc";
    }
    return "unknown_layer";
}

// Lo que el juego sabe de cada tipo de elemento, mas alla de las reglas del mapa
struct ElementTraits {
    // Edificio al que se entra: la puerta es la celda de abajo al centro
    bool hasDoor;
    // Se pisa para ir a otra area: dispara una transicion
    bool travel;
};

// Sin default, para que el compilador avise si se agrega un tipo y no se decide aca.
inline ElementTraits elementTraits(ElementType type) {
    switch (type) {
        case ElementType::Cabin:
        case ElementType::Market:
        case ElementType::CommunityCenter:
            return {.hasDoor = true, .travel = false};
        case ElementType::DungeonEntrance:
        case ElementType::Door:
        case ElementType::Exit:
        case ElementType::Stairs:
        case ElementType::HouseDoor:
            return {.hasDoor = false, .travel = true};
        case ElementType::Bed:
        case ElementType::BedBlanket:
        case ElementType::MerchantMarnie:
        case ElementType::MerchantClint:
        case ElementType::MerchantPierre:
        case ElementType::MerchantRobin:
        case ElementType::Rock:
        case ElementType::AnimalSpawn:
        case ElementType::Tree:
        case ElementType::Bush:
        case ElementType::Fence:
        case ElementType::Well:
        case ElementType::Statue:
        case ElementType::Barrel:
        case ElementType::Crate:
        case ElementType::Flowers:
        case ElementType::Lamp:
        case ElementType::Torch:
        case ElementType::Fireplace:
        case ElementType::Chest:
        case ElementType::Window:
        case ElementType::Shelf:
        case ElementType::Tv:
        case ElementType::Kitchen:
        case ElementType::Rug:
        case ElementType::RugLeft:
        case ElementType::Plant:
        case ElementType::MarketCounter:
        case ElementType::MarketShelf:
            return {.hasDoor = false, .travel = false};
    }
    return {.hasDoor = false, .travel = false};
}

struct MapArea {
    std::string id;
    std::optional<AreaType> type;
    int width = 0;
    int height = 0;
    std::optional<int> levelNumber;
    std::vector<std::vector<TileType>> tiles;
    // Posicion del area en el mapa general, en casillas. La usan el editor y el minimapa: el
    // servidor la ignora.
    int offsetX = 0;
    int offsetY = 0;

    bool operator==(const MapArea&) const = default;
};

struct MapElement {
    std::string id;
    std::string areaId;
    std::optional<ElementType> type;
    std::optional<ElementLayer> layer;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;

    bool operator==(const MapElement&) const = default;
};

struct MapCell {
    int x;
    int y;
};

// La celda de abajo al centro del edificio (solo tiene sentido si su tipo tiene puerta)
inline MapCell doorCell(const MapElement& building) {
    return {building.x + building.width / 2, building.y + building.height - 1};
}

// Las estructuras (edificios, arboles, rocas) y los NPCs tapan sus celdas; los objetos no
inline bool isSolid(const MapElement& element) {
    return element.layer == ElementLayer::Structure || element.layer == ElementLayer::Npc;
}

struct MapTransition {
    // Una transicion dirigida conecta un elemento activador con una celda de otra area.
    std::string id;
    std::string fromAreaId;
    std::string triggerElementId;
    std::string toAreaId;
    int targetX = 0;
    int targetY = 0;

    bool operator==(const MapTransition&) const = default;
};

struct MapDefinition {
    std::optional<int> formatVersion;
    std::string name;
    std::vector<MapArea> areas;
    std::vector<MapElement> elements;
    std::vector<MapTransition> transitions;

    bool operator==(const MapDefinition&) const = default;
};

}  // namespace stardew::maps
