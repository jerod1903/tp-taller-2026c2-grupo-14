#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "common/lib/thread.h"
#include "common/map/MapDefinition.h"
#include "server/game/commands/game_context.h"
#include "server/game/game_event.h"
#include "server/game/sessions.h"
#include "server/game/world/world.h"

class Game: public Thread, public GameContext {
public:
    // Arma el World a partir del mapa, que tiene que estar validado.
    Game(std::string world_id, const stardew::maps::MapDefinition& map);

    Inbox& get_inbox();

    PlayerSessions& sessions() override;
    // El Movement del World, que tiene los personajes.
    PlayerMovement& movement() override;

    void start() override;
    void run() override;
    void stop() override;

    ~Game() override;

private:
    bool started = false;
    Inbox inbox;
    std::unique_ptr<World> world;
    Sessions player_sessions;
    uint32_t tick = 0;

    // Ejecuta los comandos pendientes de la inbox sin bloquearse.
    void process_events();
    void broadcast_snapshot();
};

#endif  // GAME_H
