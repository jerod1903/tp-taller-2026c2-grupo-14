#include "server/game/characters/enemies/skull.h"

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "server/game/world/dungeon/dungeon_level.h"
#include "server/game/world/dungeon/flame.h"
#include "tests/world_helpers.h"

namespace {

const ServerConfig::Flame& flame_config() { return server_config().enemies.flame; }

constexpr uint32_t SEED = 7;
// Un tick: las llamas avanzan de a poco, como en el juego
constexpr float TICK = 0.1f;

// La calavera en (2, 2) de un nivel de 20x20, con una pared en la columna 10. `visitors` hace de
// la lista del nivel.
class SkullTest: public ::testing::Test {
protected:
    DungeonLevel level{"level", make_grid(20, 20, wall_column(10)), 0, SEED};
    std::vector<Creature*> visitors;

    static std::vector<Position> wall_column(int x) {
        std::vector<Position> walls;
        for (int y = 0; y < 20; ++y) {
            walls.push_back(Position{x, y});
        }
        return walls;
    }
};

TEST_F(SkullTest, NothingHurtsIt) {
    Skull skull(1, Position{2, 2}, level, visitors, FlameKind::Blue, SEED);
    skull.take_damage(1000);
    EXPECT_TRUE(skull.is_alive());
    EXPECT_EQ(skull.action(), EntityAction::None);
    EXPECT_FALSE(skull.is_gone());
}

TEST_F(SkullTest, StaysStillAndShootsAPlayerItSees) {
    Skull skull(1, Position{2, 2}, level, visitors, FlameKind::Blue, SEED);
    TestCreature player(level, Position{6, 2});
    visitors = {&player};
    skull.update(0);
    EXPECT_EQ(skull.get_direction(), Direction::NONE);
    EXPECT_EQ(skull.action(), EntityAction::Attacking);
    ASSERT_EQ(skull.get_flames().size(), 1u);
    EXPECT_EQ(level.get_flames().size(), 1u);
}

TEST_F(SkullTest, ShootsOncePerCooldown) {
    Skull skull(1, Position{2, 2}, level, visitors, FlameKind::Blue, SEED);
    TestCreature player(level, Position{9, 2}, 0.0f);
    visitors = {&player};
    skull.update(0);
    skull.update(server_config().enemies.skull.cooldown / 2);
    EXPECT_EQ(skull.get_flames().size(), 1u);
}

TEST_F(SkullTest, TheFlameBurnsThePlayerAndGoesOut) {
    Skull skull(1, Position{2, 2}, level, visitors, FlameKind::Blue, SEED);
    TestCreature player(level, Position{6, 2}, 0.0f);
    visitors = {&player};
    // Un segundo: la llama llega, y todavía no pasó el cooldown para tirar otra
    for (int i = 0; i < 10; ++i) {
        skull.update(TICK);
    }
    EXPECT_EQ(player.get_health(), 10 - flame_config().damage);
    EXPECT_TRUE(skull.get_flames().empty());
    EXPECT_TRUE(level.get_flames().empty());
}

// --- La llama sola ---

class FlameTest: public SkullTest {};

TEST_F(FlameTest, GoesOutAgainstAWall) {
    Flame flame(1, Point{2.5f, 2.5f}, Point{15.5f, 2.5f}, level, visitors, FlameKind::Blue);
    // La pared está a 7.5 celdas: menos que su alcance
    while (!flame.is_gone() && flame.get_x() < 10) {
        flame.update(TICK);
    }
    EXPECT_TRUE(flame.is_gone());
    EXPECT_LT(flame.get_x(), 11);
}

TEST_F(FlameTest, GoesOutAfterItsRange) {
    Flame flame(1, Point{2.5f, 2.5f}, Point{2.5f, 19.5f}, level, visitors, FlameKind::Blue);
    flame.update((flame_config().range - 1) / flame_config().speed);
    EXPECT_FALSE(flame.is_gone());
    flame.update(2 / flame_config().speed);
    EXPECT_TRUE(flame.is_gone());
}

TEST_F(FlameTest, AGreenOneIsSlowerAndBurnsTwice) {
    Flame blue(1, Point{2.5f, 2.5f}, Point{2.5f, 5.5f}, level, visitors, FlameKind::Blue);
    Flame green(2, Point{2.5f, 2.5f}, Point{2.5f, 5.5f}, level, visitors, FlameKind::Green);
    blue.update(1.0f);
    green.update(1.0f);
    EXPECT_FLOAT_EQ(blue.get_y() - 2.5f, 2 * (green.get_y() - 2.5f));
    EXPECT_EQ(green.entity_type(), EntityType::GreenFlame);
}

TEST_F(FlameTest, IsInTheLevelWhileItExists) {
    {
        Flame flame(1, Point{2.5f, 2.5f}, Point{2.5f, 5.5f}, level, visitors, FlameKind::Blue);
        EXPECT_EQ(level.get_flames().size(), 1u);
    }
    EXPECT_TRUE(level.get_flames().empty());
}

}  // namespace
