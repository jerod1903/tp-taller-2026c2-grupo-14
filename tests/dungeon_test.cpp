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
#include "server/game/world/dungeon/spawners/orc_spawner.h"
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

TEST_F(DungeonLevelTest, DeadEnemiesAreRemovedOnceTheirBodyIsGone) {
    auto enemy = std::make_unique<CountingEnemy>(level, &deaths);
    CountingEnemy& ref = *enemy;
    level.spawn(std::move(enemy));
    level.spawn(std::make_unique<Skeleton>(2, Position{1, 1}, level, NO_VISITORS, 0));
    ref.take_damage(10);
    level.update(CORPSE_TIME / 2);
    EXPECT_EQ(level.get_entities().size(), 2u);
    level.update(CORPSE_TIME);
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

// La pareja de vampiros y las calaveras de los dos colores son un solo tipo
EntityType same_type(EntityType type) {
    switch (type) {
        case EntityType::Vampiress:
            return EntityType::Vampire;
        case EntityType::GreenSkull:
            return EntityType::Skull;
        default:
            return type;
    }
}

// Si pone tantos de ese tipo como dice server_config.toml
bool expected_count(EntityType type, size_t count) {
    const ServerConfig::Enemies& config = server_config().enemies;
    switch (type) {
        case EntityType::Skeleton:
            return count == static_cast<size_t>(config.skeleton.count);
        case EntityType::Vampire:
            return count == static_cast<size_t>(2 * config.vampire.pairs);
        case EntityType::Skull:
            return count == 1;
        case EntityType::Orc:
            return count >= static_cast<size_t>(config.orc.min_group) &&
                   count <= static_cast<size_t>(config.orc.max_group);
        default:
            return false;
    }
}

TEST_F(RepopulateTest, WhenSomeoneEntersEnemiesOfASingleTypeAppearFarFromThePortals) {
    for (int entry = 0; entry < 10; ++entry) {
        level.set_visitors({});
        level.set_visitors({&player});
        const std::vector<std::unique_ptr<Enemy>>& found = level.get_enemies();
        ASSERT_FALSE(found.empty());
        EntityType type = found.front()->entity_type();
        type = same_type(type);
        EXPECT_TRUE(expected_count(type, found.size()));
        for (const std::unique_ptr<Enemy>& enemy: found) {
            EntityType other = enemy->entity_type();
            EXPECT_EQ(same_type(other), type);
            const Position& pos = enemy->get_position();
            EXPECT_GE(std::hypot(pos.x, pos.y), server_config().enemies.spawn_distance);
        }
    }
}

TEST_F(RepopulateTest, SkeletonsAppearAlone) {
    SkeletonSpawner().spawn(level);
    EXPECT_TRUE(expected_count(EntityType::Skeleton, level.get_enemies().size()));
}

TEST_F(RepopulateTest, EachVampireAppearsNextToItsPartner) {
    VampireSpawner().spawn(level);
    const std::vector<std::unique_ptr<Enemy>>& found = level.get_enemies();
    ASSERT_TRUE(expected_count(EntityType::Vampire, found.size()));
    // De a pares: el vampiro y después su vampira
    for (size_t i = 0; i + 1 < found.size(); i += 2) {
        EXPECT_EQ(found[i]->entity_type(), EntityType::Vampire);
        EXPECT_EQ(found[i + 1]->entity_type(), EntityType::Vampiress);
        const Position& a = found[i]->get_position();
        const Position& b = found[i + 1]->get_position();
        EXPECT_EQ(std::abs(a.x - b.x) + std::abs(a.y - b.y), 1);
    }
}

TEST_F(RepopulateTest, OrcsAppearInAGroupCloseToTheFirst) {
    OrcSpawner().spawn(level);
    const std::vector<std::unique_ptr<Enemy>>& found = level.get_enemies();
    ASSERT_TRUE(expected_count(EntityType::Orc, found.size()));
    const Position& first = found.front()->get_position();
    for (const std::unique_ptr<Enemy>& orc: found) {
        EXPECT_EQ(orc->entity_type(), EntityType::Orc);
        const Position& pos = orc->get_position();
        EXPECT_LE(std::max(std::abs(pos.x - first.x), std::abs(pos.y - first.y)), 2);
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

class MiningLevelTest: public ::testing::Test {
protected:
    MiningLevelTest() { level.add_portal(Portal{Position{0, 0}, &outside, Point{0.5f, 0.5f}}); }

    // Busca la primera celda que tenga el mineral pedido (después de repoblar).
    Position find_ore(DungeonLevel& target, ItemType ore) const {
        for (const auto& [pos, node]: target.get_ores()) {
            if (node.ore == ore) {
                return pos;
            }
        }
        ADD_FAILURE() << "No se generó el mineral esperado en el nivel";
        return Position{0, 0};
    }

    int count_ore(const DungeonLevel& target, ItemType ore) const {
        int total = 0;
        for (const auto& [pos, node]: target.get_ores()) {
            if (node.ore == ore) {
                ++total;
            }
        }
        return total;
    }

    TestLocation outside;
    DungeonLevel level{"level", make_grid(20, 20), 0, 7};
    TestCreature visitor{level, Position{0, 1}};
};

TEST_F(MiningLevelTest, CoalIsMinedInOneStrike) {
    level.set_visitors({&visitor});
    const Position pos = find_ore(level, ItemType::Coal);

    DungeonLevel::StrikeResult result = level.strike_ore(pos, 0.5f);
    EXPECT_EQ(result.strike, DungeonLevel::Strike::Mined);
    EXPECT_EQ(result.ore, ItemType::Coal);
    EXPECT_EQ(level.ore_at(pos), nullptr);
}

TEST_F(MiningLevelTest, LapisTakesTwoStrikesAndYieldsItemOnHighRoll) {
    level.set_visitors({&visitor});
    const Position pos = find_ore(level, ItemType::Lapis);

    DungeonLevel::StrikeResult first = level.strike_ore(pos, 0.5f);
    EXPECT_EQ(first.strike, DungeonLevel::Strike::Chipped);
    EXPECT_NE(level.ore_at(pos), nullptr);

    DungeonLevel::StrikeResult second = level.strike_ore(pos, 0.99f);
    EXPECT_EQ(second.strike, DungeonLevel::Strike::Mined);
    EXPECT_EQ(second.ore, ItemType::Lapis);
    EXPECT_EQ(level.ore_at(pos), nullptr);
}

TEST_F(MiningLevelTest, LapisCrumblesOnLowRollWithoutDroppingItem) {
    level.set_visitors({&visitor});
    const Position pos = find_ore(level, ItemType::Lapis);

    ASSERT_EQ(level.strike_ore(pos, 0.5f).strike, DungeonLevel::Strike::Chipped);

    DungeonLevel::StrikeResult second = level.strike_ore(pos, 0.0f);
    EXPECT_EQ(second.strike, DungeonLevel::Strike::Crumbled);
    EXPECT_EQ(second.ore, ItemType::Empty);
    EXPECT_EQ(level.ore_at(pos), nullptr);
}

TEST_F(MiningLevelTest, SilverAndGoldRequireConfiguredStrikes) {
    DungeonLevel deep_level{"level", make_grid(20, 20), 4, 7};
    deep_level.add_portal(Portal{Position{0, 0}, &outside, Point{0.5f, 0.5f}});
    deep_level.set_visitors({&visitor});

    const Position silver_pos = find_ore(deep_level, ItemType::Silver);
    const int silver_hits = server_config().mining.silver.hits;
    for (int i = 0; i < silver_hits - 1; ++i) {
        EXPECT_EQ(deep_level.strike_ore(silver_pos, 0.99f).strike, DungeonLevel::Strike::Chipped);
    }
    DungeonLevel::StrikeResult silver_final = deep_level.strike_ore(silver_pos, 0.99f);
    EXPECT_EQ(silver_final.strike, DungeonLevel::Strike::Mined);
    EXPECT_EQ(silver_final.ore, ItemType::Silver);

    const Position gold_pos = find_ore(deep_level, ItemType::GoldOre);
    const int gold_hits = server_config().mining.gold.hits;
    for (int i = 0; i < gold_hits - 1; ++i) {
        EXPECT_EQ(deep_level.strike_ore(gold_pos, 0.99f).strike, DungeonLevel::Strike::Chipped);
    }
    DungeonLevel::StrikeResult gold_final = deep_level.strike_ore(gold_pos, 0.99f);
    EXPECT_EQ(gold_final.strike, DungeonLevel::Strike::Mined);
    EXPECT_EQ(gold_final.ore, ItemType::GoldOre);
}

TEST_F(MiningLevelTest, StrikingEmptyCellReturnsNoOre) {
    const Position empty{2, 2};
    DungeonLevel::StrikeResult result = level.strike_ore(empty, 0.5f);
    EXPECT_EQ(result.strike, DungeonLevel::Strike::NoOre);
    EXPECT_EQ(result.ore, ItemType::Empty);
}

TEST_F(MiningLevelTest, OreBlocksWalkingUntilBroken) {
    level.set_visitors({&visitor});
    const Position pos = find_ore(level, ItemType::Coal);

    EXPECT_FALSE(level.is_walkable(pos));
    level.strike_ore(pos, 0.5f);
    EXPECT_TRUE(level.is_walkable(pos));
}

TEST_F(MiningLevelTest, GoldOnlyAppearsFromDepthFourOnward) {
    DungeonLevel depth3{"level", make_grid(20, 20), 3, 7};
    depth3.add_portal(Portal{Position{0, 0}, &outside, Point{0.5f, 0.5f}});
    depth3.set_visitors({&visitor});
    EXPECT_EQ(count_ore(depth3, ItemType::GoldOre), 0);

    DungeonLevel depth4{"level", make_grid(20, 20), 4, 7};
    depth4.add_portal(Portal{Position{0, 0}, &outside, Point{0.5f, 0.5f}});
    depth4.set_visitors({&visitor});
    EXPECT_GT(count_ore(depth4, ItemType::GoldOre), 0);
}

}  // namespace
