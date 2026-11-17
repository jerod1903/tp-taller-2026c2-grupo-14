#include "server/game/world/farm/crop.h"

Crop::Crop(CropType type): type(type) {}

uint8_t Crop::stage_count() const {
    static constexpr uint8_t COUNTS[] = {0, 6, 6, 7, 6};  // None, Strawberry, Scallion, Potato, Pumpkin
    return COUNTS[static_cast<uint8_t>(type)];
}

bool Crop::is_ready() const { return is_planted() && stage == stage_count() - 1; }

bool Crop::is_planted() const { return type != CropType::None; }

CropType Crop::get_type() const { return type; }

uint8_t Crop::get_stage() const { return stage; }
