#include "server/game/world/dungeon/dungeon.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "server/game/characters/enemies/enemy.h"
#include "server/game/characters/enemies/skeleton.h"
#include "server/game/world/dungeon/spawners/skeleton_spawner.h"
#include "server/game/world/dungeon/spawners/vampire_spawner.h"
#include "tests/world_helpers.h"

namespace {

// Vida 10, daño 1, velocidad 1, ve a 5 celdas, alcanza a 1 y pega cada 1 segundo.
const ServerConfig::Fighter STATS{10, 1, 1.0f, 5.0f, 1.0f, 1.0f};

// Nadie lo visita: los enemigos de estos tests no ven a nadie
const std::vector<Creature*> NO_VISITORS;

// Enemy que cuenta cuántas veces murió y camina hacia donde le digan.
class CountingEnemy: public Enemy {
public:
    CountingEnemy(Location& location, int* deaths):
            Enemy(1, Position{0, 0}, location, STATS, NO_VISITORS, 1.0f, 1.0f, 0), deaths(deaths) {}
    EntityType entity_type() const override { return EntityType::Skeleton; }
    void on_death() override { ++*deaths; }

protected:
    void react(float) override {}

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
    level.spawn(std::make_unique<Skeleton>(1, Position{1, 1}, level, NO_VISITORS, 0));
    EXPECT_EQ(level.get_entities().size(), 1u);
}

TEST_F(DungeonLevelTest, DeadEnemiesAreRemovedOnUpdate) {
    auto enemy = std::make_unique<CountingEnemy>(level, &deaths);
    CountingEnemy& ref = *enemy;
    level.spawn(std::move(enemy));
    level.spawn(std::make_unique<Skeleton>(2, Position{1, 1}, level, NO_VISITORS, 0));
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
    int deaths = 0;
    auto enemy = std::make_unique<CountingEnemy>(deepest, &deaths);
    enemy->set_direction(Direction::DOWN);
    CountingEnemy& ref = *enemy;
    deepest.spawn(std::move(enemy));
    dungeon.update(1.0f);
    EXPECT_NE(ref.get_position(), (Position{0, 0}));
}

// --- Repoblar al entrar ---

// Nivel de 20x20 con un portal en (0, 0): las celdas cerca de él no tienen enemigos.
class RepopulateTest: public ::testing::Test {
protected:
    RepopulateTest() { level.add_portal(Portal{Position{0, 0}, &outside, Point{0.5f, 0.5f}}); }

    // Las entidades del nivel que no son el jugador
    std::vector<const Entity*> enemies() const {
        std::vector<const Entity*> found;
        for (const Entity* entity: level.get_entities()) {
            if (entity != &player) {
                found.push_back(entity);
            }
        }
        return found;
    }

    TestLocation outside;
    DungeonLevel level{"level", make_grid(20, 20), 0, 7};
    TestCreature player{level, Position{0, 1}};
};

// Cuántos pone de ese tipo, según server_config.toml
size_t expected_count(EntityType type) {
    const ServerConfig::Enemies& config = server_config().enemies;
    return type == EntityType::Skeleton ? config.skeleton.count : 2 * config.vampire.pairs;
}

TEST_F(RepopulateTest, WhenSomeoneEntersEnemiesOfASingleTypeAppearFarFromThePortals) {
    for (int entry = 0; entry < 10; ++entry) {
        level.set_visitors({});
        level.set_visitors({&player});
        const std::vector<std::unique_ptr<Enemy>>& found = level.get_enemies();
        ASSERT_FALSE(found.empty());
        EntityType type = found.front()->entity_type();
        if (type == EntityType::Vampiress) {
            type = EntityType::Vampire;  // la pareja es un solo tipo
        }
        EXPECT_EQ(found.size(), expected_count(type));
        for (const std::unique_ptr<Enemy>& enemy: found) {
            EntityType other = enemy->entity_type();
            EXPECT_EQ(other == EntityType::Vampiress ? EntityType::Vampire : other, type);
            const Position& pos = enemy->get_position();
            EXPECT_GE(std::hypot(pos.x, pos.y), server_config().enemies.spawn_distance);
        }
    }
}

TEST_F(RepopulateTest, SkeletonsAppearAlone) {
    SkeletonSpawner().spawn(level);
    EXPECT_EQ(level.get_enemies().size(), expected_count(EntityType::Skeleton));
}

TEST_F(RepopulateTest, EachVampireAppearsNextToItsPartner) {
    VampireSpawner().spawn(level);
    const std::vector<std::unique_ptr<Enemy>>& found = level.get_enemies();
    ASSERT_EQ(found.size(), expected_count(EntityType::Vampire));
    // De a pares: el vampiro y después su vampira
    for (size_t i = 0; i + 1 < found.size(); i += 2) {
        EXPECT_EQ(found[i]->entity_type(), EntityType::Vampire);
        EXPECT_EQ(found[i + 1]->entity_type(), EntityType::Vampiress);
        const Position& a = found[i]->get_position();
        const Position& b = found[i + 1]->get_position();
        EXPECT_EQ(std::abs(a.x - b.x) + std::abs(a.y - b.y), 1);
    }
}

TEST_F(RepopulateTest, OnlyAnEmptyLevelRepopulates) {
    level.set_visitors({&player});
    std::vector<uint32_t> first;
    for (const Entity* entity: level.get_entities()) {
        first.push_back(entity->get_id());
    }
    // Sigue el mismo jugador: no cambian
    level.set_visitors({&player});
    std::vector<uint32_t> same;
    for (const Entity* entity: level.get_entities()) {
        same.push_back(entity->get_id());
    }
    EXPECT_EQ(same, first);
    // Se va y vuelve: son otros
    level.set_visitors({});
    level.set_visitors({&player});
    for (const Entity* entity: level.get_entities()) {
        EXPECT_EQ(std::count(first.begin(), first.end(), entity->get_id()), 0);
    }
}

}  // namespace
