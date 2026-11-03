#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "common/lib/thread.h"
#include "common/map/MapDefinition.h"
#include "server/game/game_event.h"
#include "server/game/world/world.h"

class Game: public Thread {
public:
    // Arma el World a partir del mapa, que tiene que estar validado.
    Game(std::string world_id, const stardew::maps::MapDefinition& map);

    Inbox& get_inbox();

    void start() override;
    void run() override;
    void stop() override;

    ~Game() override;

private:
    bool started = false;
    const std::string world_id;
    Inbox inbox;
    std::unique_ptr<World> world;
    std::map<uint32_t, std::shared_ptr<Outbox>> players;
    uint32_t tick = 0;

    void process_events();
    void handle(const PlayerJoined& joined);
    void handle(const PlayerAction& action);
    // Cierra la outbox: el Sender termina de vaciarla y ya no le llega nada más.
    void handle(const PlayerLeft& left);
    void broadcast_snapshot();
    // Con la outbox llena o cerrada el mensaje se descarta: el Game nunca se bloquea.
    static void send(Outbox& outbox, const GameMessage& message);
};

#endif  // GAME_H
