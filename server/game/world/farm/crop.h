#ifndef CROP_H
#define CROP_H

#include <cstdint>

#include "common/farm_dto.h"

// Lo que hay en una celda cavada de la granja. CropType::None es tierra cavada sin plantar.
class Crop {
public:
    explicit Crop(CropType type = CropType::None);

    uint8_t stage_count() const;  // 6, 6, 7, 6 según el tipo
    bool is_ready() const;        // stage == stage_count() - 1
    bool is_planted() const;
    // más adelante: riegos por día, días para crecer, precio base
    CropType get_type() const;
    uint8_t get_stage() const;

private:
    CropType type;
    uint8_t stage = 0;
};

#endif  // CROP_H
