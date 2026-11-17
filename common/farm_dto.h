#ifndef FARM_DTO_H
#define FARM_DTO_H

#include <cstdint>
#include <string>
#include <vector>

#include "common/game_dto.h"

enum class CropType : uint8_t {
    None = 0,  // tierra cavada sin plantar
    Strawberry,
    Scallion,
    Potato,
    Pumpkin,
};

struct CropHarvest {
    CropType type;
    ItemQuality quality;

    bool operator==(const CropHarvest&) const = default;
};


// Una celda cavada de la granja. x y y son celdas, no milésimas.
struct FarmTile {
    uint16_t x;
    uint16_t y;
    CropType crop = CropType::None;
    uint8_t stage = 0;        // etapa de crecimiento; la última es "lista para cosechar"
    bool needsWater = false;  // se dibuja el icono de riego
    bool readyToHarvest = false;  // se dibuja el icono de cosecha
    
    bool operator==(const FarmTile&) const = default;
};

// Todas las celdas cavadas del área areaId. Se manda completo cada vez que algo cambia, a todos los
// jugadores de la partida. Una celda que no está en la lista no está cavada.
// En el cable: tag GameMsg::FarmState, areaId, u16 cantidad y, por cada celda,
// u16 col, u16 row, u8 crop, u8 stage, u8 needsWater, u8 readyToHarvest.
struct FarmState {
    std::string areaId;
    std::vector<FarmTile> tiles = {};

    bool operator==(const FarmState&) const = default;
};


#endif  // FARM_DTO_H