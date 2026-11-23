#include "server/game/characters/enemies/skeleton.h"

#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "tests/world_helpers.h"

namespace {

const ServerConfig::Skeleton& config() { return server_config().enemies.skeleton; }

constexpr uint32_t SEED = 7;
const std::vector<Creature*> NOBODY;

// Hacia dónde deambula un esqueleto igual (misma semilla, en (2, 2)) que no ve a nadie: el que no
// ve al jugador tiene que hacer lo mismo.
Direction wandering_direction(Location& where) {
    Skeleton alone(1, Position{2, 2}, where, NOBODY, SEED);
    alone.update(0);
    return alone.get_direction();
}

// El esqueleto en (2, 2) de un nivel de 20x20 sin paredes. `visitors` hace de la lista del nivel.
class SkeletonTest: public ::testing::Test {
protected:
    TestLocation level{make_grid(20, 20)};
    std::vector<Creature*> visitors;
    Skeleton skeleton{1, Position{2, 2}, level, visitors, SEED};
};

TEST_F(SkeletonTest, WalksTowardsAPlayerItSees) {
    TestCreature player(level, Position{6, 2});
    visitors = {&player};
    skeleton.update(0);
    EXPECT_EQ(skeleton.get_direction(), Direction::RIGHT);
}

TEST_F(SkeletonTest, DoesNotSeeThroughWalls) {
    std::vector<Position> wall;
    for (int y = 0; y < 20; ++y) {
        wall.push_back(Position{4, y});
    }
    TestLocation walled(make_grid(20, 20, wall));
    Skeleton behind_wall(1, Position{2, 2}, walled, visitors, SEED);
    TestCreature player(walled, Position{6, 2});
    visitors = {&player};
    behind_wall.update(0);
    EXPECT_EQ(behind_wall.get_direction(), wandering_direction(walled));
}

TEST_F(SkeletonTest, DoesNotSeeFurtherThanItsSight) {
    TestCreature player(level, Position{2 + static_cast<int>(config().fighter.sight) + 2, 2});
    visitors = {&player};
    skeleton.update(0);
    EXPECT_EQ(skeleton.get_direction(), wandering_direction(level));
}

TEST_F(SkeletonTest, WandersWhileItSeesNobody) {
    std::set<Direction> seen;
    for (int i = 0; i < 60; ++i) {  // un minuto, de a un segundo
        skeleton.update(1.0f);
        seen.insert(skeleton.get_direction());
    }
    EXPECT_GT(seen.size(), 2u);
}

TEST_F(SkeletonTest, HitsRightAwayAndThenWaitsBetweenHits) {
    TestCreature player(level, Position{3, 2});
    visitors = {&player};
    int health = player.get_health();
    skeleton.update(0);
    EXPECT_EQ(player.get_health(), health - config().fighter.damage);
    // Todavía no pasó el tiempo entre golpes
    skeleton.update(config().fighter.cooldown / 2);
    EXPECT_EQ(player.get_health(), health - config().fighter.damage);
    skeleton.update(config().fighter.cooldown);
    EXPECT_EQ(player.get_health(), health - 2 * config().fighter.damage);
}

TEST_F(SkeletonTest, LeavesAPlayerWithNoHealthAlone) {
    TestCreature player(level, Position{3, 2});
    player.take_damage(player.get_health());
    visitors = {&player};
    skeleton.update(0);
    EXPECT_EQ(skeleton.get_direction(), wandering_direction(level));
}

TEST_F(SkeletonTest, RevivesWithFullHealthAfterAWhile) {
    skeleton.take_damage(config().fighter.health);
    EXPECT_FALSE(skeleton.is_alive());
    EXPECT_FALSE(skeleton.is_gone());
    skeleton.update(config().revive_seconds / 2);
    EXPECT_FALSE(skeleton.is_alive());
    skeleton.update(config().revive_seconds);
    EXPECT_EQ(skeleton.get_health(), config().fighter.health);
}

// Revive y vuelve a morir: no vuelve a sortear oro, así no se lo puede farmear
TEST_F(SkeletonTest, OnlyItsFirstDeathDropsGold) {
    skeleton.take_hit(config().fighter.health);
    skeleton.update(config().revive_seconds * 2);
    ASSERT_TRUE(skeleton.is_alive());
    EXPECT_EQ(skeleton.take_hit(config().fighter.health), 0);
}

// --- Lo que está haciendo, para que el cliente lo anime ---

TEST_F(SkeletonTest, DoesNothingUntilItSeesSomeone) {
    skeleton.update(0);
    EXPECT_EQ(skeleton.action(), EntityAction::None);
}

TEST_F(SkeletonTest, IsAttackingForAWhileAfterAHit) {
    TestCreature player(level, Position{3, 2});
    visitors = {&player};
    skeleton.update(0);
    EXPECT_EQ(skeleton.action(), EntityAction::Attacking);
    // Antes del próximo golpe ya terminó de pegar
    skeleton.update(config().fighter.cooldown * 0.9f);
    EXPECT_EQ(skeleton.action(), EntityAction::None);
}

TEST_F(SkeletonTest, IsHurtForAWhileAfterBeingHit) {
    skeleton.take_damage(1);
    EXPECT_EQ(skeleton.action(), EntityAction::Hurt);
    skeleton.update(1.0f);
    EXPECT_EQ(skeleton.action(), EntityAction::None);
}

TEST_F(SkeletonTest, BeingHurtShowsOverAttacking) {
    TestCreature player(level, Position{3, 2});
    visitors = {&player};
    skeleton.update(0);
    skeleton.take_damage(1);
    EXPECT_EQ(skeleton.action(), EntityAction::Hurt);
}

TEST_F(SkeletonTest, IsDeadUntilItRevivesAndThenDoesNothing) {
    skeleton.take_damage(config().fighter.health);
    EXPECT_EQ(skeleton.action(), EntityAction::Dead);
    skeleton.update(config().revive_seconds);
    EXPECT_EQ(skeleton.action(), EntityAction::None);
}

}  // namespace
