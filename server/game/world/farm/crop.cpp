#include "server/game/world/farm/crop.h"

#include <algorithm>
#include <array>
#include <utility>

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

Crop::Crop(const SavedCrop& saved):
        type(saved.type),
        days_grown(saved.days_grown),
        window(saved.window),
        watered(saved.watered),
        window_start(saved.window_start),
        skipped(saved.skipped),
        watering_delay(saved.watering_delay),
        waterings(saved.waterings),
        skipped_total(saved.skipped_total),
        days_ready(saved.days_ready) {}

SavedCrop Crop::saved() const {
    return SavedCrop{type,    days_grown,     window,    watered,       window_start,
                     skipped, watering_delay, waterings, skipped_total, days_ready};
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

ItemType Crop::product() const {
    switch (type) {
        case CropType::Strawberry:
            return ItemType::Strawberry;
        case CropType::Scallion:
            return ItemType::Scallion;
        case CropType::Potato:
            return ItemType::Potato;
        case CropType::Pumpkin:
            return ItemType::Pumpkin;
        case CropType::None:
            break;
    }
    return ItemType::Empty;
}

ItemQuality Crop::quality(float minute, float luck) const {
    const ServerConfig::Quality& config = server_config().crops.quality;
    float weight = config.watering_weight;
    float score = (weight * watering_score() + (1.0f - weight) * harvest_score(minute)) * luck;
    // De la mejor a la peor: la primera cuyo mínimo alcanza
    const std::array<std::pair<float, ItemQuality>, 4> levels = {{
            {config.perfect, ItemQuality::Perfect},
            {config.incredible, ItemQuality::Incredible},
            {config.excellent, ItemQuality::Excellent},
            {config.good, ItemQuality::Good},
    }};
    auto level = std::find_if(levels.begin(), levels.end(),
                              [score](const auto& level) { return score >= level.first; });
    return level != levels.end() ? level->second : ItemQuality::Base;
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
    if (is_ready()) {
        ++days_ready;
    } else if (growing()) {
        skipped += watered ? 0 : 1;  // la última ventana del día
        skipped_total += skipped;
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
        watering_delay = 0.0f;
        waterings = 0;
        skipped_total = 0;
        days_ready = 0;
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

float Crop::watering_score() const {
    int windows = waterings + skipped_total;
    if (windows == 0) {
        return 1.0f;
    }
    float window_length = MINUTES_PER_DAY / server_config().crops.of(type).waterings_per_day;
    // Cada riego suma su demora como fracción de la ventana; cada salteado, una ventana entera
    float lateness = watering_delay / window_length + static_cast<float>(skipped_total);
    return std::clamp(1.0f - lateness / static_cast<float>(windows), 0.0f, 1.0f);
}

float Crop::harvest_score(float minute) const {
    float waited = static_cast<float>(days_ready) * MINUTES_PER_DAY + minute;
    float grace = server_config().crops.quality.harvest_grace_days * MINUTES_PER_DAY;
    return std::clamp(1.0f - waited / grace, 0.0f, 1.0f);
}
