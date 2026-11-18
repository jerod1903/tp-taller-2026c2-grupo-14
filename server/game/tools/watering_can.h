#ifndef WATERING_CAN_H
#define WATERING_CAN_H

#include "server/game/tools/tool.h"

// Riega el cultivo de la celda de adelante, si está en la granja y lo necesita.
class WateringCan: public Tool {
public:
    bool use(Player& player, GameContext& context) const override;
    int energy_cost() const override;
};

#endif  // WATERING_CAN_H
