#include "client/audio/sonidos_granja.h"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <utility>

namespace {
std::optional<Sonido> sonido_de(const FarmTile& antes, const FarmTile& despues) {
    if (antes.soil != despues.soil) {
        return Sonido::Azada;
    }
    if (antes.crop == CropType::None && despues.crop != CropType::None) {
        return Sonido::Plantar;
    }
    if (antes.needsWater && !despues.needsWater && antes.crop == despues.crop &&
        antes.stage == despues.stage) {
        return Sonido::Regar;
    }
    return std::nullopt;
}
}  // namespace

std::set<Sonido> sonidos_granja(const FarmState& antes, const FarmState& despues) {
    std::set<Sonido> sonidos;
    if (antes.areaId != despues.areaId) {
        return sonidos;
    }
    std::map<std::pair<uint16_t, uint16_t>, FarmTile> previas;
    for (const FarmTile& celda: antes.tiles) {
        previas.emplace(std::make_pair(celda.x, celda.y), celda);
    }
    for (const FarmTile& celda: despues.tiles) {
        auto previa = previas.find({celda.x, celda.y});
        if (previa == previas.end()) {
            sonidos.insert(Sonido::Azada);
        } else if (std::optional<Sonido> sonido = sonido_de(previa->second, celda)) {
            sonidos.insert(*sonido);
        }
    }
    return sonidos;
}
