#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "common/lib/thread.h"
#include "server/game/game_event.h"

class Game: public Thread {
public:
    explicit Game(std::string world_id);

    Inbox& get_inbox();

    void start() override;
    void run() override;
    void stop() override;

    ~Game() override;

private:
    bool started = false;
    const std::string world_id;
    Inbox inbox;
    std::map<uint32_t, std::shared_ptr<Outbox>> players;
    uint32_t tick = 0;

    void process_events();
    void handle(const PlayerJoined& joined);
    void handle(const PlayerAction& action);
    void handle(const PlayerLeft& left);
    void broadcast_snapshot();
    // Con la outbox llena o cerrada el mensaje se descarta: el Game nunca se bloquea.
    static void send(Outbox& outbox, const GameMessage& message);
};

#endif  // GAME_H
