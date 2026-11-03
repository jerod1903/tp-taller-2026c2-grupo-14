#include "server/game/game.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "common/logger.h"
#include "common/operations.h"
#include "server/consts.h"
#include "tests/test_worlds.h"

namespace {

constexpr const char* WORLD_ID = "stardew";
constexpr unsigned int OUTBOX_SIZE = 64;

class GameTest: public ::testing::Test {
protected:
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        game.start();
    }

    void TearDown() override { Logger::set_level(LogLevel::Info); }

    std::shared_ptr<Outbox> join(uint32_t player_id, unsigned int outbox_size = OUTBOX_SIZE) {
        auto outbox = std::make_shared<Outbox>(outbox_size);
        game.get_inbox().push(PlayerJoined{player_id, GameMember{}, outbox});
        return outbox;
    }

    static Snapshot next_snapshot(Outbox& outbox) {
        GameMessage message = outbox.pop();
        EXPECT_TRUE(std::holds_alternative<Snapshot>(message));
        return std::get<Snapshot>(message);
    }

    static std::vector<uint32_t> ids_of(const Snapshot& snapshot) {
        std::vector<uint32_t> ids;
        for (const PlayerState& player: snapshot.players) {
            ids.push_back(player.playerId);
        }
        return ids;
    }

    void move(uint32_t player_id, int8_t dir_x, int8_t dir_y) {
        game.get_inbox().push(PlayerAction{player_id, Request{Operation::Move, dir_x, dir_y}});
    }

    // Saltea snapshots viejos hasta uno que cumpla la condición.
    template <typename Condition>
    static Snapshot wait_snapshot(Outbox& outbox, Condition condition) {
        Snapshot snapshot = next_snapshot(outbox);
        while (!condition(snapshot)) {
            snapshot = next_snapshot(outbox);
        }
        return snapshot;
    }

    Game game{WORLD_ID, example_map()};
};

TEST_F(GameTest, JoinedPlayerReceivesTheLocationFirst) {
    std::shared_ptr<Outbox> outbox = join(0);
    GameMessage message = outbox->pop();
    ASSERT_TRUE(std::holds_alternative<LocationData>(message));
    EXPECT_EQ(std::get<LocationData>(message).worldId, WORLD_ID);
}

TEST_F(GameTest, SnapshotsListTheConnectedPlayers) {
    std::shared_ptr<Outbox> first = join(0);
    join(1);
    first->pop();
    Snapshot snapshot =
            wait_snapshot(*first, [](const Snapshot& s) { return s.players.size() == 2; });
    EXPECT_EQ(ids_of(snapshot), (std::vector<uint32_t>{0, 1}));
}

// En el mapa de ejemplo el spawn_point de la granja está en (14, 6), con pasto a la derecha.
TEST_F(GameTest, JoinedPlayerStartsStillOnTheSpawnOfTheFarm) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    PlayerState player = next_snapshot(*outbox).players.at(0);
    EXPECT_EQ(player.areaId, "granja");
    EXPECT_EQ(player.x, 14);
    EXPECT_EQ(player.y, 6);
    EXPECT_EQ(player.dirX, 0);
    EXPECT_EQ(player.dirY, 0);
    EXPECT_EQ(player.health, ServerConstants::INITIAL_HEALTH);
    EXPECT_EQ(player.gold, static_cast<uint32_t>(ServerConstants::INITIAL_GOLD));
}

TEST_F(GameTest, MoveMakesThePlayerWalkThatWay) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    move(0, 1, 0);
    Snapshot snapshot =
            wait_snapshot(*outbox, [](const Snapshot& s) { return s.players.at(0).x == 15; });
    EXPECT_EQ(snapshot.players.at(0).dirX, 1);
    EXPECT_EQ(snapshot.players.at(0).y, 6);
}

TEST_F(GameTest, MoveZeroStopsThePlayer) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    move(0, 1, 0);
    wait_snapshot(*outbox, [](const Snapshot& s) { return s.players.at(0).x == 15; });
    move(0, 0, 0);
    PlayerState stopped = wait_snapshot(*outbox, [](const Snapshot& s) {
                              return s.players.at(0).dirX == 0;
                          }).players.at(0);
    EXPECT_EQ(next_snapshot(*outbox).players.at(0), stopped);
}

TEST_F(GameTest, TickIncreasesBetweenSnapshots) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    uint32_t first = next_snapshot(*outbox).tick;
    EXPECT_GT(next_snapshot(*outbox).tick, first);
}

TEST_F(GameTest, LeftPlayerIsNoLongerInTheSnapshots) {
    std::shared_ptr<Outbox> stays = join(0);
    join(1);
    game.get_inbox().push(PlayerLeft{1});
    stays->pop();
    Snapshot snapshot =
            wait_snapshot(*stays, [](const Snapshot& s) { return s.players.size() == 1; });
    EXPECT_EQ(ids_of(snapshot), (std::vector<uint32_t>{0}));
}

TEST_F(GameTest, LeftPlayerOutboxIsClosedAfterItsPendingMessages) {
    std::shared_ptr<Outbox> outbox = join(0);
    game.get_inbox().push(PlayerLeft{0});
    EXPECT_TRUE(std::holds_alternative<LocationData>(outbox->pop()));
    try {
        for (int i = 0; i < 1000; ++i) {
            outbox->pop();
        }
        FAIL() << "la outbox sigue abierta";
    } catch (const ClosedQueue&) {}
}

TEST_F(GameTest, ActionsDoNotStopTheLoop) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    game.get_inbox().push(PlayerAction{0, Request{Operation::Move, 1, 0}});
    uint32_t first = next_snapshot(*outbox).tick;
    EXPECT_GT(next_snapshot(*outbox).tick, first);
}

TEST_F(GameTest, AFullOutboxDoesNotBlockTheOthers) {
    join(0, 1);  // Nadie la vacía: se llena con el LocationData.
    std::shared_ptr<Outbox> other = join(1);
    other->pop();
    uint32_t first = next_snapshot(*other).tick;
    EXPECT_GT(next_snapshot(*other).tick, first);
}

TEST_F(GameTest, AClosedOutboxIsIgnored) {
    join(0)->close();
    std::shared_ptr<Outbox> other = join(1);
    other->pop();
    uint32_t first = next_snapshot(*other).tick;
    EXPECT_GT(next_snapshot(*other).tick, first);
}

TEST_F(GameTest, StopEndsTheThread) {
    game.stop();
    for (int i = 0; i < 200 && game.is_alive(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_FALSE(game.is_alive());
}

}  // namespace
