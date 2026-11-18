#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/characters/player.h"
#include "server/game/commands/player_characters.h"
#include "server/game/commands/player_farming.h"
#include "server/game/world/direction.h"
#include "server/game/world/dungeon/dungeon.h"
#include "server/game/world/farm/farm.h"
#include "server/game/world/movement.h"
#include "server/game/world/town/town.h"
#include "server/game/world/game_clock.h"

// El mapa completo de una partida. Es dueño de las tres zonas y de los personajes de los
// jugadores, que se piden por id. Los portales entre zonas (pueblo <-> granja, pueblo <-> mazmorra)
// los enlaza quien arma el mundo a partir del mapa, porque las posiciones de origen y llegada salen
// de ahí. Atiende a los comandos que usan los personajes (PlayerCharacters) y la granja
// (PlayerFarming).
class World: public PlayerCharacters, public PlayerFarming {
public:
    World(std::unique_ptr<Farm> farm, std::unique_ptr<Town> town, Dungeon dungeon);

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;

    Farm& get_farm();
    Town& get_town();
    Dungeon& get_dungeon();

    // Crea su personaje en su cama de la cabaña, con los valores de server_config.toml. Si ya
    // estaba, lo reemplaza.
    void spawn_player(uint32_t player_id, std::string name);
    // Lo da de alta en su Location actual, reemplazando al que tenga su mismo id.
    Player& add_player(std::unique_ptr<Player> player);
    // Si no está, no hace nada.
    void remove_player(uint32_t player_id);
    Player* find_player(uint32_t player_id) override;
    bool dig(const Player& player) override;
    bool plant(const Player& player, CropType crop) override;
    bool water(const Player& player) override;
    const Outcome& harvest(Player& player) override;
    const Outcome& milk(Player& player) override;
    // Atiende a MoveCommand.
    Movement& get_movement();

    // Lanza std::out_of_range si el jugador no está.
    const Location& location_of(uint32_t player_id) const;
    const std::map<uint32_t, std::unique_ptr<Player>>& get_players() const;
    const std::vector<std::unique_ptr<Animal>>& get_animals() const;
    // Con las camas que ya tienen dueño.
    const Cabin& get_cabin() const;
    const GameClock& get_clock() const;

    // Actualiza las tres zonas, hace viajar a los que pisaron un portal y termina el día a las
    // 2:00 o cuando caen todos inconscientes.
    void update(float dt);

private:
    std::unique_ptr<Farm> farm;
    std::unique_ptr<Town> town;
    Dungeon dungeon;
    std::map<uint32_t, std::unique_ptr<Player>> players;
    Movement movement;
    GameClock clock{server_config().game.day_speed};

    // Despierta a todos en su cama: enteros los que durmieron en ella, a medias los demás.
    void start_new_day();
    // El día termina antes si cayeron todos.
    bool everyone_unconscious() const;
};

#endif  // WORLD_H
