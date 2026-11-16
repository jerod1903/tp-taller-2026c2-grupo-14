#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>

#include "server/game/commands/player_movement.h"
#include "server/game/world/direction.h"
#include "server/game/world/portal.h"
#include "server/game/world/position.h"
#include "server/game/world/spawn_point.h"

class Farm;
class Player;

// Decide donde aparece cada jugador, hacia donde camina y cuando viaja por un portal. Como se mueve
// dentro de una Location (velocidad, paredes) lo resuelve Creature. Atiende a MoveCommand.
class Movement: public PlayerMovement {
public:
    // `players` son los personajes del World, por id.
    Movement(Farm& farm, const std::map<uint32_t, std::unique_ptr<Player>>& players);

    // Su cama en la cabaña: la primera vez se le asigna una.
    SpawnPoint spawn_point(uint32_t player_id);

    void move_player(uint32_t player_id, Direction dir) override;

    // Los que en este tick entraron a la celda de un portal aparecen en su llegada.
    void travel_through_portals();

    // Los que quedaron encerrados (apareció algo donde estaban parados) pasan a la celda libre más
    // cercana de su Location.
    void free_trapped();

private:
    Farm& farm;
    const std::map<uint32_t, std::unique_ptr<Player>>& players;

    static void travel(Player& player, const Portal& portal);
    // Recorre a lo ancho desde `from`; std::nullopt si no hay ninguna celda libre.
    static std::optional<Position> nearest_free_cell(const Location& location,
                                                     const Position& from);
};

#endif  // MOVEMENT_H
