#include "server/game/game.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "common/logger.h"
#include "server/config/server_config.h"
#include "server/game/commands/join_command.h"
#include "server/game/commands/leave_command.h"
#include "server/game/commands/move_command.h"
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
        return join_as(player_id, GameMember{}, outbox_size);
    }

    std::shared_ptr<Outbox> join_as(uint32_t player_id, const GameMember& member,
                                    unsigned int outbox_size = OUTBOX_SIZE) {
        auto outbox = std::make_shared<Outbox>(outbox_size);
        game.get_inbox().push(std::make_shared<JoinCommand>(player_id, member, outbox));
        return outbox;
    }

    // Saltea los mensajes de otro tipo hasta el próximo T.
    template <typename T>
    static T next(Outbox& outbox) {
        GameMessage message = outbox.pop();
        while (!std::holds_alternative<T>(message)) {
            message = outbox.pop();
        }
        return std::get<T>(message);
    }

    static Snapshot next_snapshot(Outbox& outbox) { return next<Snapshot>(outbox); }

    static std::vector<uint32_t> ids_of(const Snapshot& snapshot) {
        std::vector<uint32_t> ids;
        for (const PlayerState& player: snapshot.players) {
            ids.push_back(player.playerId);
        }
        return ids;
    }

    void move(uint32_t player_id, int8_t dir_x, int8_t dir_y) {
        game.get_inbox().push(std::make_shared<MoveCommand>(player_id, MoveRequest{dir_x, dir_y}));
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

TEST_F(GameTest, JoinedPlayerReceivesItsOwnInfoAfterTheLocation) {
    const GameMember member{"jero", "Martu", CharacterAppearance{5, 2, 7, 1, 4}};
    std::shared_ptr<Outbox> outbox = join_as(0, member);
    outbox->pop();
    GameMessage message = outbox->pop();
    ASSERT_TRUE(std::holds_alternative<PlayerInfo>(message));
    EXPECT_EQ(std::get<PlayerInfo>(message), (PlayerInfo{0, "Martu", member.appearance}));
}

TEST_F(GameTest, SecondPlayerReceivesTheInfoOfBoth) {
    const GameMember first{"jero", "Martu", CharacterAppearance{5, 2, 7, 1, 4}};
    const GameMember second{"santi", "Santu", CharacterAppearance{1, 0, 2, 3, 0}};
    join_as(0, first);
    std::shared_ptr<Outbox> outbox = join_as(1, second);
    outbox->pop();
    // Llegan juntos, antes de cualquier snapshot: el orden entre ellos no está definido.
    std::vector<PlayerInfo> infos{next<PlayerInfo>(*outbox), next<PlayerInfo>(*outbox)};
    std::sort(infos.begin(), infos.end(),
              [](const PlayerInfo& a, const PlayerInfo& b) { return a.playerId < b.playerId; });
    EXPECT_EQ(infos, (std::vector<PlayerInfo>{{0, "Martu", first.appearance},
                                              {1, "Santu", second.appearance}}));
}

TEST_F(GameTest, FirstPlayerReceivesTheInfoOfTheSecond) {
    const GameMember second{"santi", "Santu", CharacterAppearance{1, 0, 2, 3, 0}};
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    ASSERT_EQ(next<PlayerInfo>(*outbox).playerId, 0u);  // el suyo
    join_as(1, second);
    EXPECT_EQ(next<PlayerInfo>(*outbox), (PlayerInfo{1, "Santu", second.appearance}));
}

TEST_F(GameTest, EachSnapshotHasAPlayerEntityPerPlayerWithItsDirection) {
    std::shared_ptr<Outbox> outbox = join(0);
    join(1);
    move(1, 1, 0);
    Snapshot snapshot = wait_snapshot(*outbox, [](const Snapshot& s) {
        return s.players.size() == 2 && s.entities.at(1).dirX != 0;
    });
    ASSERT_EQ(snapshot.players.size(), 2u);
    // los jugadores van primero; despues vienen los animales
    for (std::size_t i = 0; i < snapshot.players.size(); ++i) {
        const EntityState& entity = snapshot.entities[i];
        const PlayerState& player = snapshot.players[i];
        EXPECT_EQ(entity.type, EntityType::Player);
        EXPECT_EQ(entity.entityId, player.playerId);
        EXPECT_EQ(entity.areaId, player.areaId);
        EXPECT_EQ(entity.x, player.x);
        EXPECT_EQ(entity.y, player.y);
    }
    EXPECT_EQ(snapshot.entities[0].dirX, 0);
    EXPECT_EQ(snapshot.entities[1].dirX, 1);
    EXPECT_EQ(snapshot.entities[1].dirY, 0);
}

TEST_F(GameTest, SnapshotsHaveTheAnimalsOfTheFarm) {
    std::shared_ptr<Outbox> outbox = join(0);
    Snapshot snapshot = next_snapshot(*outbox);
    int cows = 0;
    int chickens = 0;
    for (const EntityState& entity: snapshot.entities) {
        cows += entity.type == EntityType::Cow ? 1 : 0;
        chickens += entity.type == EntityType::Chicken ? 1 : 0;
        if (entity.type != EntityType::Player) {
            EXPECT_EQ(entity.areaId, "granja");
        }
    }
    EXPECT_EQ(cows, server_config().animals.cows);
    EXPECT_EQ(chickens, server_config().animals.chickens);
}

TEST_F(GameTest, SnapshotsListTheConnectedPlayers) {
    std::shared_ptr<Outbox> first = join(0);
    join(1);
    first->pop();
    Snapshot snapshot =
            wait_snapshot(*first, [](const Snapshot& s) { return s.players.size() == 2; });
    EXPECT_EQ(ids_of(snapshot), (std::vector<uint32_t>{0, 1}));
}

// En el mapa de ejemplo el spawn_point de la granja está en (14, 7), frente a la puerta de la
// cabaña, con pasto a la derecha.
TEST_F(GameTest, JoinedPlayerStartsStillOnTheSpawnOfTheFarm) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    PlayerState player = next_snapshot(*outbox).players.at(0);
    EXPECT_EQ(player.areaId, "granja");
    EXPECT_EQ(player.x, 14 * POSITION_SCALE + POSITION_SCALE / 2);  // centro de la celda
    EXPECT_EQ(player.y, 7 * POSITION_SCALE + POSITION_SCALE / 2);
    EXPECT_EQ(player.dirX, 0);
    EXPECT_EQ(player.dirY, 0);
    EXPECT_EQ(player.health, server_config().player.health);
    EXPECT_EQ(player.gold, static_cast<uint32_t>(server_config().player.gold));
}

TEST_F(GameTest, MoveMakesThePlayerWalkThatWay) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    move(0, 1, 0);
    Snapshot snapshot = wait_snapshot(
            *outbox, [](const Snapshot& s) { return s.players.at(0).x / POSITION_SCALE == 15; });
    EXPECT_EQ(snapshot.players.at(0).dirX, 1);
    EXPECT_EQ(snapshot.players.at(0).y, 7 * POSITION_SCALE + POSITION_SCALE / 2);
}

TEST_F(GameTest, MoveCanGoDiagonally) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    move(0, 1, 1);
    PlayerState player = wait_snapshot(*outbox, [](const Snapshot& s) {
                             return s.players.at(0).dirX != 0;
                         }).players.at(0);
    EXPECT_EQ(player.dirX, 1);
    EXPECT_EQ(player.dirY, 1);
}

TEST_F(GameTest, MoveZeroStopsThePlayer) {
    std::shared_ptr<Outbox> outbox = join(0);
    outbox->pop();
    move(0, 1, 0);
    wait_snapshot(*outbox,
                  [](const Snapshot& s) { return s.players.at(0).x / POSITION_SCALE == 15; });
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
    game.get_inbox().push(std::make_shared<LeaveCommand>(1));
    stays->pop();
    Snapshot snapshot =
            wait_snapshot(*stays, [](const Snapshot& s) { return s.players.size() == 1; });
    EXPECT_EQ(ids_of(snapshot), (std::vector<uint32_t>{0}));
}

TEST_F(GameTest, LeftPlayerOutboxIsClosedAfterItsPendingMessages) {
    std::shared_ptr<Outbox> outbox = join(0);
    game.get_inbox().push(std::make_shared<LeaveCommand>(0));
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
    move(0, 1, 0);
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
