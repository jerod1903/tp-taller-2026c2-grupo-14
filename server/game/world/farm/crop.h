#ifndef CROP_H
#define CROP_H

#include <cstdint>

#include "common/farm_dto.h"

// Lo que hay en una celda cavada de la granja. CropType::None es tierra cavada sin plantar.
// El día se parte en waterings_per_day ventanas y en cada una necesita un riego. Al empezar el
// día siguiente crece un día solo si no salteó más de skippable_waterings riegos. Los números
// salen de [crops.*] en server_config.toml.
class Crop {
public:
    // minute: la hora a la que se planta, en minutos de juego desde las 6:00.
    explicit Crop(CropType type = CropType::None, float minute = 0.0f);

    uint8_t stage_count() const;  // etapas del sprite: 6, 6, 7, 6 según el tipo
    bool is_planted() const;
    bool is_ready() const;  // pasaron days_to_grow días desde que se plantó (o volvió a dar)
    CropType get_type() const;
    // Reparte los días de crecimiento entre las etapas del sprite; la última es "lista".
    uint8_t get_stage() const;
    // Plantado, creciendo y sin regar en la ventana actual.
    bool needs_water() const;

    // Riega la ventana actual. false si no lo necesitaba.
    bool water(float minute);
    // Avanza la hora: si se cerró una ventana sin riego, cuenta como salteado. true si cambió
    // needs_water().
    bool update_time(float minute);
    // Empieza el día: crece si no salteó de más, y vuelve a la primera ventana.
    void grow();
    // Después de cosecharlo: si vuelve a dar (la frutilla), retrocede regrow_days días; si no,
    // la celda queda sin plantar.
    void harvested();

private:
    CropType type;
    int days_grown = 0;
    int window = 0;  // ventana de riego actual
    bool watered = false;
    float window_start = 0.0f;  // minuto en que se abrió la ventana actual
    int skipped = 0;            // ventanas de hoy que se cerraron sin riego
    // Para la calidad: cuánto se tardó en regar, sumado sobre todos los riegos
    float watering_delay = 0.0f;
    int waterings = 0;

    int days_to_grow() const;
    int window_of(float minute) const;
    bool growing() const;
};

#endif  // CROP_H
