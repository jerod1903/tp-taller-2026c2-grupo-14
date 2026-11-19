#include "server/game/characters/enemies/vampire.h"

#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "tests/world_helpers.h"

namespace {

const ServerConfig::Vampire& config() { return server_config().enemies.vampire; }

constexpr uint32_t SEED = 7;

// La pareja en (2, 2) y (3, 2) de un nivel de 20x20 sin paredes, y un blanco aparte para medir
// cuánto pegan. `visitors` hace de la lista del nivel.
class VampireTest: public ::testing::Test {
protected:
    VampireTest() { vampire.pair_with(vampiress); }

    // La vida que le saca `attacker` de un golpe
    int hit(const Vampire& attacker) {
        int health = target.get_health();
        attacker.attack(target);
        return health - target.get_health();
    }

    void kill(Vampire& who) { who.take_damage(who.get_health()); }

    TestLocation level{make_grid(20, 20)};
    std::vector<Creature*> visitors;
    Vampire vampire{1, Position{2, 2}, level, visitors, VampireKind::Vampire, SEED};
    Vampire vampiress{2, Position{3, 2}, level, visitors, VampireKind::Vampiress, SEED};
    Vampire target{3, Position{15, 15}, level, visitors, VampireKind::Vampire, SEED};
};

TEST_F(VampireTest, EachOneIsItsOwnKind) {
    EXPECT_EQ(vampire.entity_type(), EntityType::Vampire);
    EXPECT_EQ(vampiress.entity_type(), EntityType::Vampiress);
}

TEST_F(VampireTest, TheVampiressIsFasterAndTheVampireHitsHarder) {
    EXPECT_EQ(vampiress.speed(), config().fast_speed);
    EXPECT_EQ(vampire.speed(), config().fighter.speed);
    EXPECT_EQ(hit(vampire), config().strong_damage);
    EXPECT_EQ(hit(vampiress), config().fighter.damage);
}

TEST_F(VampireTest, WhenTheVampireDiesTheVampiressGoesTwiceAsFast) {
    kill(vampire);
    EXPECT_TRUE(vampiress.is_in_frenzy());
    EXPECT_EQ(vampiress.speed(), 2 * config().fast_speed);
}

TEST_F(VampireTest, WhenTheVampiressDiesTheVampireHitsTwiceAsHard) {
    kill(vampiress);
    EXPECT_TRUE(vampire.is_in_frenzy());
    EXPECT_EQ(hit(vampire), 2 * config().strong_damage);
}

TEST_F(VampireTest, ADeadVampireStaysDeadWithoutBlood) {
    kill(vampire);
    vampire.update(10 * config().revive_seconds);
    EXPECT_FALSE(vampire.is_alive());
    EXPECT_FALSE(vampire.is_gone());
}

TEST_F(VampireTest, BloodNearItsBodyRevivesItAfterAWhile) {
    kill(vampire);
    TestCreature player(level, Position{2, 3});
    player.take_damage(player.get_health());  // murió al lado
    visitors = {&player};
    vampire.update(0);
    vampire.update(config().revive_seconds / 2);
    EXPECT_FALSE(vampire.is_alive());
    vampire.update(config().revive_seconds);
    EXPECT_EQ(vampire.get_health(), config().fighter.health);
}

TEST_F(VampireTest, BloodFarFromItsBodyDoesNotCount) {
    kill(vampire);
    TestCreature player(level, Position{2 + static_cast<int>(config().blood_reach) + 2, 2});
    player.take_damage(player.get_health());
    visitors = {&player};
    vampire.update(0);
    vampire.update(10 * config().revive_seconds);
    EXPECT_FALSE(vampire.is_alive());
}

TEST_F(VampireTest, WhenItRevivesItsPartnerCalmsDown) {
    kill(vampire);
    TestCreature player(level, Position{2, 3});
    player.take_damage(player.get_health());
    visitors = {&player};
    vampire.update(0);
    vampire.update(config().revive_seconds);
    EXPECT_TRUE(vampire.is_alive());
    EXPECT_FALSE(vampiress.is_in_frenzy());
    EXPECT_FALSE(vampire.is_in_frenzy());
}

}  // namespace
