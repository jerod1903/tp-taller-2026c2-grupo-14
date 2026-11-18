#ifndef CROP_H
#define CROP_H

#include <cstdint>

#include "common/farm_dto.h"

// Lo que hay en una celda cavada de la granja. CropType::None es tierra cavada sin plantar.
// Crece de a un día; los días y si vuelve a dar salen de [crops.*] en server_config.toml.
class Crop {
public:
    explicit Crop(CropType type = CropType::None);

    uint8_t stage_count() const;  // etapas del sprite: 6, 6, 7, 6 según el tipo
    bool is_planted() const;
    bool is_ready() const;  // pasaron days_to_grow días desde que se plantó (o volvió a dar)
    CropType get_type() const;
    // Reparte los días de crecimiento entre las etapas del sprite; la última es "lista".
    uint8_t get_stage() const;

    // Pasa un día. Lista, no crece más.
    void grow();
    // Después de cosecharlo: si vuelve a dar (la frutilla), retrocede regrow_days días; si no,
    // la celda queda sin plantar.
    void harvested();

private:
    CropType type;
    int days_grown = 0;

    int days_to_grow() const;
};

#endif  // CROP_H
