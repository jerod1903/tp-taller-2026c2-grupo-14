#include "server/game/world/farm/crop.h"

#include <algorithm>

#include "server/config/server_config.h"

Crop::Crop(CropType type): type(type) {}

uint8_t Crop::stage_count() const {
    static constexpr uint8_t COUNTS[] = {0, 6, 6, 7, 6};  // None, Strawberry, Scallion, Potato, Pumpkin
    return COUNTS[static_cast<uint8_t>(type)];
}

bool Crop::is_planted() const { return type != CropType::None; }

bool Crop::is_ready() const { return is_planted() && days_grown >= days_to_grow(); }

CropType Crop::get_type() const { return type; }

uint8_t Crop::get_stage() const {
    if (!is_planted()) {
        return 0;
    }
    int last = stage_count() - 1;
    if (is_ready()) {
        return static_cast<uint8_t>(last);
    }
    // Mientras crece no llega a la última etapa
    return static_cast<uint8_t>(std::min(last - 1, days_grown * last / days_to_grow()));
}

void Crop::grow() {
    if (is_planted() && !is_ready()) {
        ++days_grown;
    }
}

void Crop::harvested() {
    int regrow = server_config().crops.of(type).regrow_days;
    if (regrow > 0) {
        days_grown = days_to_grow() - regrow;
    } else {
        *this = Crop{};
    }
}

int Crop::days_to_grow() const { return server_config().crops.of(type).days_to_grow; }
