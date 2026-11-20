#ifndef MILKING_H
#define MILKING_H

#include <memory>
#include <vector>

#include "server/game/characters/animals/animal.h"
#include "server/game/characters/player.h"
#include "server/game/interactions/outcome.h"

// Ordeña la vaca lista más cercana al jugador dentro del alcance, de su misma área, y le da la
// leche. Si no le entra en el inventario, la vaca no se ordeña.
const Outcome& milk_nearest_cow(const std::vector<std::unique_ptr<Animal>>& animals,
                                Player& player);

#endif  // MILKING_H
