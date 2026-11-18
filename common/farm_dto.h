#ifndef FARM_DTO_H
#define FARM_DTO_H

#include <cstdint>
#include <string>
#include <vector>

// La calidad de un producto. También la usa el inventario (game_dto.h incluye este archivo): los
// objetos se apilan solo con los de su mismo tipo y calidad.
enum class ItemQuality : uint8_t {
    Base,
    Good,
    Excellent,
    Incredible,
    Perfect,
};

// Lo que puede haber en un slot del inventario o tirado en el piso de la granja.
enum class ItemType : uint8_t {
    Empty = 0,
    Pickaxe,
    WateringCan,
    Shovel,
    Sword,
    Milk,
    StrawberrySeeds,
    ScallionSeeds,
    PotatoSeeds,
    PumpkinSeeds,
    Strawberry,
    Scallion,
    Potato,
    Pumpkin,
    Egg,
};

enum class SoilType : uint8_t { Dirt, Tilled };

enum class CropType : uint8_t {
    None = 0,  // tierra cavada sin plantar
    Strawberry,
    Scallion,
    Potato,
    Pumpkin,
};

// Una celda cavada/cavable de la granja. x y y son celdas, no milésimas.
struct FarmTile {
    uint16_t x;
    uint16_t y;
    SoilType soil;
    CropType crop = CropType::None;
    uint8_t stage = 0;        // etapa de crecimiento; la última es "lista para cosechar"
    bool needsWater = false;  // se dibuja el icono de riego
    bool readyToHarvest = false;  // se dibuja el icono de cosecha
    
    bool operator==(const FarmTile&) const = default;
};

// Un objeto tirado en el piso de la granja (un huevo), en la celda x, y.
struct GroundItem {
    uint16_t x;
    uint16_t y;
    ItemType item;

    bool operator==(const GroundItem&) const = default;
};

// Las celdas trabajadas con la pala y los objetos del piso del área areaId. Se manda completo cada
// vez que algo cambia, a todos los jugadores de la partida. Una celda que no está en la lista no
// se cavó.
// En el cable: tag GameMsg::FarmState, areaId, u16 cantidad y, por cada celda,
// u16 x, u16 y, u8 soil, u8 crop, u8 stage, u8 needsWater, u8 readyToHarvest; después u16
// cantidad y, por cada objeto, u16 x, u16 y, u8 item.
struct FarmState {
    std::string areaId;
    std::vector<FarmTile> tiles = {};
    std::vector<GroundItem> items = {};

    bool operator==(const FarmState&) const = default;
};


#endif  // FARM_DTO_H