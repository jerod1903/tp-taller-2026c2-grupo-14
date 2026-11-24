#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>

#include "common/lib/thread.h"
#include "common/map/MapDefinition.h"
#include "server/game/commands/game_context.h"
#include "server/game/commerce/market.h"
#include "server/game/day_vote.h"
#include "server/game/game_event.h"
#include "server/game/sessions.h"
#include "server/game/storage/chest_access.h"
#include "server/game/world/world.h"
#include "server/persistence/game_store.h"
#include "server/persistence/saved_game.h"

class Game: public Thread, public GameContext, public PlayerDay {
public:
    // Arma el World a partir del mapa, que tiene que estar validado, y lo retoma desde `saved` si
    // la partida ya tenía algo guardado. Cada vez que empieza un día lo guarda en `store` (desde
    // su hilo: el store no comparte nada entre partidas).
    Game(std::string world_id, const stardew::maps::MapDefinition& map, uint32_t game_id,
         GameStore& store, const std::optional<SavedGame>& saved = std::nullopt);

    Inbox& get_inbox();

    PlayerSessions& sessions() override;
    // El Movement del World, que tiene los personajes.
    PlayerMovement& movement() override;
    // El World, dueño de los personajes y los animales.
    PlayerCharacters& characters() override;
    // Los comerciantes del mercado.
    PlayerCommerce& commerce() override;
    // El World, dueño de la granja.
    PlayerFarming& farming() override;
    PlayerCombat& combat() override;
    // La votación de fin de día: la lleva el Game.
    PlayerDay& day() override;
    // Quién tiene abierto qué cofre; los cofres son de cada Location.
    PlayerStorage& storage() override;
    void ready_for_next_day(uint32_t player_id) override;
    void end_day() override;

    void start() override;
    void run() override;
    void stop() override;

    ~Game() override;

private:
    bool started = false;
    uint32_t game_id;
    GameStore& store;
    Inbox inbox;
    std::unique_ptr<World> world;
    Market market;       // después de world: usa su granja y su mercado
    ChestAccess chests;  // después de world: busca a sus jugadores
    Sessions player_sessions;
    uint32_t tick = 0;
    // Mientras se vota pasar de día: el mundo congelado
    std::optional<DayVote> vote;

    // Ejecuta los comandos pendientes de la inbox sin bloquearse.
    void process_events();
    void broadcast_snapshot();
    // Terminó el día: congela el mundo y empieza a votar.
    void open_vote();
    // Suma o saca de la votación a los que entraron o se fueron; si votaron todos, arranca el día
    // siguiente y guarda.
    void update_vote();
    // A cada conectado, cómo va la votación (con si durmió él).
    void send_vote();
    std::set<uint32_t> connected_players() const;
    // Guarda la partida. Si no puede, lo loguea y sigue: se vuelve a intentar al otro día.
    void save();
};

#endif  // GAME_H
