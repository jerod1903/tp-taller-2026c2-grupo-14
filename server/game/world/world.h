#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <vector>

#include "server/game/world/dungeon/dungeon.h"
#include "server/game/world/farm/farm.h"
#include "server/game/world/portal.h"
#include "server/game/world/town/town.h"

class Player;

// El mapa completo de una partida. Es dueño de las tres zonas. Los portales entre zonas (pueblo <->
// granja, pueblo <-> mazmorra) los enlaza quien arma el mundo a partir del mapa, porque las
// posiciones de origen y llegada salen de ahí.
class World {
public:
    World(std::unique_ptr<Farm> farm, std::unique_ptr<Town> town, Dungeon dungeon);

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;

    Farm& get_farm();
    Town& get_town();
    Dungeon& get_dungeon();

    // Lo da de alta (o de baja) también en su Location actual.
    void add_player(Player& player);
    void remove_player(const Player& player);

    Location& location_of(const Player& player) const;

    // Saca al jugador de su Location actual y lo deja en portal.arrival dentro de portal.target
    // (debe estar parado sobre portal.origin).
    void travel(Player& player, const Portal& portal);

    // Actualiza las tres zonas y después llama a check_portals().
    void update(float dt);

private:
    std::unique_ptr<Farm> farm;
    std::unique_ptr<Town> town;
    Dungeon dungeon;
    std::vector<Player*> players;

    // Para cada jugador que en este tick entró a la celda de un portal, llama a travel().
    void check_portals();
};

#endif  // WORLD_H
