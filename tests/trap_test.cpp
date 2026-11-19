#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "server/game/world/dungeon/dungeon_level.h"
#include "server/game/world/dungeon/traps/flamethrower.h"
#include "server/game/world/dungeon/traps/spikes.h"
#include "server/game/world/dungeon/traps/trap_spawner.h"
#include "tests/world_helpers.h"

namespace {

const ServerConfig::Traps& config() { return server_config().traps; }

// Un nivel de 10x10 con una pared en (5, 2). `targets` hace de la lista de jugadores del nivel.
class TrapTest: public ::testing::Test {
protected:
    TestLocation level{make_grid(10, 10, {Position{5, 2}})};
    std::vector<Creature*> targets;
};

TEST_F(TrapTest, ActiveSpikesHurtWhoIsOnTheirCellOncePerActivation) {
    Spikes spikes(1, Position{2, 2}, level, targets, 0);
    TestCreature player(level, Position{2, 2});
    targets = {&player};
    spikes.update(0);
    spikes.update(config().spikes.active_time / 2);
    EXPECT_TRUE(spikes.is_active());
    EXPECT_EQ(player.get_health(), 10 - config().spikes.damage);
}

TEST_F(TrapTest, InactiveSpikesDoNotHurt) {
    // Arranca justo cuando se esconden
    Spikes spikes(1, Position{2, 2}, level, targets, config().spikes.active_time);
    TestCreature player(level, Position{2, 2});
    targets = {&player};
    spikes.update(0);
    EXPECT_FALSE(spikes.is_active());
    EXPECT_EQ(player.get_health(), 10);
}

TEST_F(TrapTest, SpikesHurtAgainOnTheNextActivation) {
    Spikes spikes(1, Position{2, 2}, level, targets, 0);
    TestCreature player(level, Position{2, 2});
    targets = {&player};
    spikes.update(0);
    spikes.update(config().spikes.active_time + config().spikes.idle_time);
    EXPECT_EQ(player.get_health(), 10 - 2 * config().spikes.damage);
}

TEST_F(TrapTest, SpikesDoNotHurtNextToThem) {
    Spikes spikes(1, Position{2, 2}, level, targets, 0);
    TestCreature player(level, Position{3, 2});
    targets = {&player};
    spikes.update(0);
    EXPECT_EQ(player.get_health(), 10);
}

TEST_F(TrapTest, TheFlamethrowerBurnsUpToItsReach) {
    Flamethrower flamethrower(1, Position{2, 0}, level, targets, Direction::DOWN, 0);
    TestCreature inside(level, Position{2, config().flamethrower.reach - 1});
    TestCreature outside(level, Position{2, config().flamethrower.reach});
    targets = {&inside, &outside};
    flamethrower.update(0);
    EXPECT_EQ(inside.get_health(), 10 - config().flamethrower.trap.damage);
    EXPECT_EQ(outside.get_health(), 10);
}

TEST_F(TrapTest, TheFlamethrowerStopsAtAWall) {
    // La pared está en (5, 2): el que está detrás no se quema
    Flamethrower flamethrower(1, Position{4, 2}, level, targets, Direction::RIGHT, 0);
    TestCreature behind(level, Position{6, 2});
    targets = {&behind};
    flamethrower.update(0);
    EXPECT_EQ(behind.get_health(), 10);
}

TEST_F(TrapTest, EachFlamethrowerDirectionIsItsOwnType) {
    EXPECT_EQ(Flamethrower(1, Position{2, 0}, level, targets, Direction::DOWN, 0).entity_type(),
              EntityType::FlamethrowerDown);
    EXPECT_EQ(Flamethrower(1, Position{0, 2}, level, targets, Direction::RIGHT, 0).entity_type(),
              EntityType::FlamethrowerRight);
    EXPECT_EQ(Flamethrower(1, Position{9, 2}, level, targets, Direction::LEFT, 0).entity_type(),
              EntityType::FlamethrowerLeft);
}

// --- Al azar en el nivel ---

TEST(TrapSpawnerTest, PutsTheTrapsAndEachFlamethrowerAgainstAWall) {
    DungeonLevel level{"level", make_grid(20, 20), 0, 7};
    TrapSpawner().spawn(level);
    int spikes = 0;
    int flamethrowers = 0;
    for (const std::unique_ptr<Trap>& trap: level.get_traps()) {
        const Position& pos = trap->get_position();
        switch (trap->entity_type()) {
            case EntityType::Spikes:
                ++spikes;
                break;
            case EntityType::FlamethrowerDown:
                ++flamethrowers;
                EXPECT_FALSE(level.is_walkable(step(pos, Direction::UP)));
                break;
            case EntityType::FlamethrowerRight:
                ++flamethrowers;
                EXPECT_FALSE(level.is_walkable(step(pos, Direction::LEFT)));
                break;
            case EntityType::FlamethrowerLeft:
                ++flamethrowers;
                EXPECT_FALSE(level.is_walkable(step(pos, Direction::RIGHT)));
                break;
            default:
                ADD_FAILURE();
        }
    }
    EXPECT_EQ(spikes, config().spikes_count);
    EXPECT_EQ(flamethrowers, config().flamethrowers_count);
}

TEST(TrapSpawnerTest, NoFlamethrowerHangsFromADecoration) {
    // Un piso de 5x5 rodeado de decoración (piso tapado, como una antorcha o un barril) y sin
    // ninguna pared: no hay dónde colgar un lanzallamas
    const int size = 7;
    std::vector<Tile> tiles(size * size, Tile(TileType::CaveFloor, true));
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (x == 0 || y == 0 || x == size - 1 || y == size - 1) {
                tiles[y * size + x] = Tile(TileType::CaveFloor, false);
            }
        }
    }
    DungeonLevel level{"level", Grid(size, size, std::move(tiles)), 0, 7};
    TrapSpawner().spawn(level);
    for (const std::unique_ptr<Trap>& trap: level.get_traps()) {
        EXPECT_EQ(trap->entity_type(), EntityType::Spikes);
    }
}

}  // namespace
