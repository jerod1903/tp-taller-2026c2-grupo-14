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

    // Saltea snapshots viejos hasta uno que cumpla la condición.
    template <typename Condition>
    static Snapshot wait_snapshot(Outbox& outbox, Condition condition) {
        Snapshot snapshot = next_snapshot(outbox);
        while (!condition(snapshot)) {
            snapshot = next_snapshot(outbox);
        }
        return snapshot;
    }

    Game game{WORLD_ID};
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
    EXPECT_EQ(snapshot.players, (std::vector<uint32_t>{0, 1}));
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
    EXPECT_EQ(snapshot.players, (std::vector<uint32_t>{0}));
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
