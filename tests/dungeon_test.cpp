#include "server/game/world/dungeon/dungeon.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "server/game/characters/enemies/enemy.h"
#include "server/game/characters/enemies/skeleton.h"
#include "tests/world_helpers.h"

namespace {

// Enemy que cuenta cuántas veces murió.
class CountingEnemy: public Enemy {
public:
    CountingEnemy(Location& location, int* deaths):
            Enemy(1, Position{0, 0}, location, 10, 1.0f), deaths(deaths) {}
    void on_death() override { ++*deaths; }
    void update(float dt) override { advance(dt); }

private:
    int* deaths;
};

std::vector<std::unique_ptr<DungeonLevel>> make_levels(int count) {
    std::vector<std::unique_ptr<DungeonLevel>> levels;
    for (int depth = 0; depth < count; ++depth) {
        levels.push_back(std::make_unique<DungeonLevel>("level", make_grid(), depth));
    }
    return levels;
}

class DungeonLevelTest: public ::testing::Test {
protected:
    DungeonLevel level{"level", make_grid(), 0};
    int deaths = 0;
};

TEST_F(DungeonLevelTest, OnDeathIsCalledOnlyOnce) {
    CountingEnemy enemy(level, &deaths);
    enemy.take_damage(10);
    enemy.take_damage(10);
    EXPECT_EQ(deaths, 1);
}

TEST_F(DungeonLevelTest, SpawnRegistersTheEnemy) {
    level.spawn(std::make_unique<Skeleton>(1, Position{1, 1}, level));
    EXPECT_EQ(level.get_entities().size(), 1u);
}

TEST_F(DungeonLevelTest, DeadEnemiesAreRemovedOnUpdate) {
    auto enemy = std::make_unique<CountingEnemy>(level, &deaths);
    CountingEnemy& ref = *enemy;
    level.spawn(std::move(enemy));
    level.spawn(std::make_unique<Skeleton>(2, Position{1, 1}, level));
    ref.take_damage(10);
    level.update(0.1f);
    ASSERT_EQ(level.get_entities().size(), 1u);
    EXPECT_EQ(level.get_entities()[0]->get_id(), 2u);
}

TEST(DungeonTest, RequiresExactlyLevelCountLevels) {
    EXPECT_THROW(Dungeon(make_levels(Dungeon::LEVEL_COUNT - 1)), std::invalid_argument);
    EXPECT_NO_THROW(Dungeon(make_levels(Dungeon::LEVEL_COUNT)));
}

TEST(DungeonTest, EntryLevelIsTheShallowest) {
    Dungeon dungeon(make_levels(Dungeon::LEVEL_COUNT));
    EXPECT_EQ(dungeon.entry_level().get_depth(), 0);
}

TEST(DungeonTest, LevelsAreIndexedByDepth) {
    Dungeon dungeon(make_levels(Dungeon::LEVEL_COUNT));
    EXPECT_EQ(dungeon.level(Dungeon::LEVEL_COUNT - 1).get_depth(), Dungeon::LEVEL_COUNT - 1);
    EXPECT_THROW(dungeon.level(Dungeon::LEVEL_COUNT), std::out_of_range);
}

TEST(DungeonTest, UpdateUpdatesEveryLevel) {
    Dungeon dungeon(make_levels(Dungeon::LEVEL_COUNT));
    DungeonLevel& deepest = dungeon.level(Dungeon::LEVEL_COUNT - 1);
    auto skeleton = std::make_unique<Skeleton>(1, Position{0, 0}, deepest);
    skeleton->set_direction(Direction::DOWN);
    Skeleton& ref = *skeleton;
    deepest.spawn(std::move(skeleton));
    dungeon.update(1.0f);
    EXPECT_NE(ref.get_position(), (Position{0, 0}));
}

}  // namespace
