#ifndef SEED_H
#define SEED_H

#include "common/farm_dto.h"
#include "server/game/tools/tool.h"

// Planta su cultivo en la celda de adelante, si es tierra arada vacía de la granja, y gasta una
// semilla.
class Seed: public Tool {
public:
    explicit Seed(CropType crop);

    bool use(Player& player, GameContext& context) const override;
    int energy_cost() const override;

private:
    CropType crop;
};

#endif  // SEED_H
