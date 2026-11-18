#include "server/game/world/farm/crop.h"

#include <algorithm>

#include "server/config/server_config.h"

namespace {
constexpr float MINUTES_PER_DAY = 1200.0f;  // de 6:00 a 2:00
}  // namespace

Crop::Crop(CropType type, float minute): type(type) {
    if (is_planted()) {
        window = window_of(minute);
        window_start = minute;
    }
}

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

bool Crop::needs_water() const { return growing() && !watered; }

bool Crop::water(float minute) {
    if (!needs_water()) {
        return false;
    }
    watered = true;
    watering_delay += minute - window_start;
    ++waterings;
    return true;
}

bool Crop::update_time(float minute) {
    int now = window_of(minute);
    if (!growing() || now == window) {
        return false;
    }
    bool needed = needs_water();
    // La que se cierra, más las que se salteó entera si pasó mucho tiempo de golpe
    skipped += (watered ? 0 : 1) + (now - window - 1);
    window = now;
    watered = false;
    window_start = minute;
    return needed != needs_water();
}

void Crop::grow() {
    if (growing()) {
        skipped += watered ? 0 : 1;  // la última ventana del día
        if (skipped <= server_config().crops.of(type).skippable_waterings) {
            ++days_grown;
        }
    }
    window = 0;
    watered = false;
    window_start = 0.0f;
    skipped = 0;
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

int Crop::window_of(float minute) const {
    if (!is_planted()) {
        return 0;
    }
    int windows = server_config().crops.of(type).waterings_per_day;
    return std::clamp(static_cast<int>(minute * windows / MINUTES_PER_DAY), 0, windows - 1);
}

bool Crop::growing() const { return is_planted() && !is_ready(); }
