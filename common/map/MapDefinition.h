#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace stardew::maps {

enum class AreaType {
    Farm,
    Village,
    DungeonLevel,
};

enum class ElementType {
    Cabin,
    Market,
    CommunityCenter,
    DungeonEntrance,
    MerchantMarnie,
    MerchantClint,
    MerchantPierre,
    MerchantRobin,
    OreNode,
    Door,
    Exit,
    Stairs,
    SpawnPoint,
    Tree,
    Bush,
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
    }
    return "unknown_tile";
}

// Agua, paredes de cueva y piedra no se pueden pisar. Lo usan el servidor y el validador.
inline bool isWalkableTerrain(TileType type) {
    return type != TileType::Water && type != TileType::CaveWall && type != TileType::Stone;
}

inline std::vector<TileType> allTileTypes() {
    return {
            TileType::Grass,     TileType::Dirt,  TileType::TilledSoil, TileType::Water,
            TileType::Path,      TileType::Stone, TileType::CaveFloor,  TileType::CaveWall,
            TileType::WoodFloor, TileType::Sand,
    };
}

inline std::optional<AreaType> areaTypeFromString(std::string_view value) {
    if (value == "farm")
        return AreaType::Farm;
    if (value == "village")
        return AreaType::Village;
    if (value == "dungeon_level")
        return AreaType::DungeonLevel;
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
    }
    return "unknown_area";
}

inline std::optional<ElementType> elementTypeFromString(std::string_view value) {
    if (value == "cabin")
        return ElementType::Cabin;
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
    if (value == "ore_node")
        return ElementType::OreNode;
    if (value == "door")
        return ElementType::Door;
    if (value == "exit")
        return ElementType::Exit;
    if (value == "stairs")
        return ElementType::Stairs;
    if (value == "spawn_point")
        return ElementType::SpawnPoint;
    if (value == "tree")
        return ElementType::Tree;
    if (value == "bush")
        return ElementType::Bush;
    return std::nullopt;
}

inline std::string_view toString(ElementType type) {
    switch (type) {
        case ElementType::Cabin:
            return "cabin";
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
        case ElementType::OreNode:
            return "ore_node";
        case ElementType::Door:
            return "door";
        case ElementType::Exit:
            return "exit";
        case ElementType::Stairs:
            return "stairs";
        case ElementType::SpawnPoint:
            return "spawn_point";
        case ElementType::Tree:
            return "tree";
        case ElementType::Bush:
            return "bush";
    }
    return "unknown_element";
}

inline std::vector<ElementType> allElementTypes() {
    return {
            ElementType::Cabin,
            ElementType::Market,
            ElementType::CommunityCenter,
            ElementType::DungeonEntrance,
            ElementType::MerchantMarnie,
            ElementType::MerchantClint,
            ElementType::MerchantPierre,
            ElementType::MerchantRobin,
            ElementType::OreNode,
            ElementType::Door,
            ElementType::Exit,
            ElementType::Stairs,
            ElementType::SpawnPoint,
            ElementType::Tree,
            ElementType::Bush,
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
            return {.hasDoor = false, .travel = true};
        case ElementType::MerchantMarnie:
        case ElementType::MerchantClint:
        case ElementType::MerchantPierre:
        case ElementType::MerchantRobin:
        case ElementType::OreNode:
        case ElementType::SpawnPoint:
        case ElementType::Tree:
        case ElementType::Bush:
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

// Las estructuras y los NPCs tapan sus celdas; los objetos (rocas, cofres) no
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
