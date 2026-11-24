#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/characters/player.h"
#include "server/game/commands/player_characters.h"
#include "server/game/commands/player_combat.h"
#include "server/game/commands/player_farming.h"
#include "server/game/world/direction.h"
#include "server/game/world/dungeon/dungeon.h"
#include "server/game/world/farm/farm.h"
#include "server/game/world/game_clock.h"
#include "server/game/world/movement.h"
#include "server/game/world/town/town.h"
#include "server/persistence/saved_game.h"

// El mapa completo de una partida. Es dueño de las tres zonas y de los personajes de los
// jugadores, que se piden por id. Los portales entre zonas (pueblo <-> granja, pueblo <-> mazmorra)
// los enlaza quien arma el mundo a partir del mapa, porque las posiciones de origen y llegada salen
// de ahí. Atiende a los comandos que usan los personajes (PlayerCharacters), la granja
// (PlayerFarming) y la pelea (PlayerCombat).
class World: public PlayerCharacters, public PlayerFarming, public PlayerCombat {
public:
    World(std::unique_ptr<Farm> farm, std::unique_ptr<Town> town, Dungeon dungeon);

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;

    Farm& get_farm();
    Town& get_town();
    Dungeon& get_dungeon();
    const Dungeon& get_dungeon() const;

    // Lo pone en su cama de la cabaña. Si ya había estado en esta partida (se fue y vuelve),
    // con todo lo que tenía; si no, un personaje nuevo con los valores de server_config.toml o,
    // si estaba en la partida guardada, con lo que tenía guardado.
    void spawn_player(uint32_t player_id, std::string name);
    // Lo da de alta en su Location actual, reemplazando al que tenga su mismo id (esté o no).
    Player& add_player(std::unique_ptr<Player> player);
    // Se fue de la partida: sale del mapa pero el mundo lo sigue teniendo, por si vuelve. Si no
    // está, no hace nada.
    void remove_player(uint32_t player_id);
    Player* find_player(uint32_t player_id) override;
    Player* player_at(const Location& location, const Position& cell) override;
    bool dig(const Player& player, const Position& cell) override;
    bool plant(const Player& player, const Position& cell, CropType crop) override;
    bool water(const Player& player, const Position& cell) override;
    const Outcome& harvest(Player& player) override;
    const Outcome& milk(Player& player) override;
    bool attack(Player& player, const Position& cell) override;
    // Atiende a MoveCommand.
    Movement& get_movement();

    // Lanza std::out_of_range si el jugador no está.
    const Location& location_of(uint32_t player_id) const;
    const std::map<uint32_t, std::unique_ptr<Player>>& get_players() const;
    const std::vector<std::unique_ptr<Animal>>& get_animals() const;
    // Con las camas que ya tienen dueño.
    const Cabin& get_cabin() const;
    const GameClock& get_clock() const;

    // Lo que se guarda de la partida: el día, la granja y los jugadores (los que están, los que se
    // fueron y los de la partida guardada que todavía no volvieron).
    SavedGame saved() const;
    // Retoma una partida guardada: el día, la granja y lo de cada jugador, que lo recupera al
    // entrar. Se llama recién creado el mundo, antes de que entre nadie.
    void restore(const SavedGame& saved);

    // Actualiza las tres zonas, hace viajar a los que pisaron un portal y termina el día a las
    // 2:00 o cuando caen todos inconscientes. true si en este update terminó el día: ahí queda
    // anotado quién durmió en su cama, y el día siguiente arranca recién con start_new_day().
    bool update(float dt);
    // Termina el día ya, como si fueran las 2:00 (el cheat $sleep): el próximo update lo cierra.
    void end_day();
    // Si el jugador estaba en su cama (y consciente) cuando terminó el último día.
    bool slept_in_bed(uint32_t player_id) const;
    // Arranca el día siguiente: despiertan todos en su cama, enteros los que durmieron en ella y a
    // medias los demás (y los que se fueron).
    void start_new_day();

private:
    std::unique_ptr<Farm> farm;
    std::unique_ptr<Town> town;
    Dungeon dungeon;
    // Los que están jugando
    std::map<uint32_t, std::unique_ptr<Player>> players;
    // Los que se fueron de la partida: fuera del mapa, con todo lo suyo, por si vuelven. No se
    // mueven ni se dibujan; al pasar el día descansan como el que no llegó a su cama.
    std::map<uint32_t, std::unique_ptr<Player>> away;
    // Los de la partida guardada, como estaban: el que entra por primera vez en esta sesión
    // recupera lo suyo de acá
    std::map<uint32_t, SavedPlayer> stored;
    // Los que estaban en su cama cuando terminó el día
    std::set<uint32_t> sleepers;
    Movement movement;
    GameClock clock{server_config().game.day_speed};

    // Si está en la granja y `cell` está pegada a la suya (también en diagonal).
    bool reaches(const Player& player, const Position& cell) const;
    // El día termina antes si cayeron todos.
    bool everyone_unconscious() const;
};

#endif  // WORLD_H
