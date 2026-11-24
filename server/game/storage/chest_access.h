#ifndef CHEST_ACCESS_H
#define CHEST_ACCESS_H

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "server/game/commands/player_characters.h"
#include "server/game/commands/player_storage.h"
#include "server/game/world/location.h"
#include "server/game/world/position.h"

// Quién tiene abierto qué cofre. Los cofres son de la Location, comunes a todos: acá solo se anota
// quién los está mirando, para validar lo que pide y avisarles a todos cuando cambian.
class ChestAccess: public PlayerStorage {
public:
    explicit ChestAccess(PlayerCharacters& characters);

    std::optional<ChestState> open(Player& player) override;
    std::optional<ChestUpdate> transfer(Player& player, ChestSide from, uint8_t slot) override;
    std::optional<ChestUpdate> split(Player& player, ChestSide side, uint8_t slot) override;
    void close(const Player& player) override;

    // Cierra los cofres de quienes ya no pueden usarlos: se fueron de la partida, cambiaron de
    // Location, se alejaron, quedaron inconscientes o el cofre ya no está. Devuelve a los que
    // siguen en la partida, para avisarles.
    std::vector<uint32_t> close_unreachable();

private:
    // Por celda y no por puntero al Chest: si el cofre se saca, no queda colgando.
    struct OpenChest {
        Location* location;
        Position cell;

        bool operator==(const OpenChest&) const = default;
    };

    PlayerCharacters& characters;
    std::map<uint32_t, OpenChest> open_chests;

    // El cofre que tiene abierto el jugador, si sigue ahí y a su alcance; si no, nullptr.
    Chest* chest_of(const Player& player);
    // Todos los que tienen abierto ese cofre.
    std::vector<uint32_t> viewers_of(const OpenChest& chest) const;
};

#endif  // CHEST_ACCESS_H
