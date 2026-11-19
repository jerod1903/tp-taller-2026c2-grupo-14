#include "server/game/world/world.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "server/game/characters/enemies/skeleton.h"
#include "server/game/characters/player.h"
#include "server/game/snapshot_builder.h"
#include "server/game/world/dungeon/dungeon.h"
#include "server/game/world/farm/farm.h"
#include "server/game/world/town/town.h"
#include "tests/world_helpers.h"

namespace {

const Position CABIN_POS{4, 4};  // Puerta de la cabaña, en la granja.
const Position DOOR_POS{0, 4};   // Puerta de salida, dentro de la cabaña.

// `beds`: los lugares para camas de la cabaña (para spawn_player)
std::unique_ptr<World> make_world(std::vector<Position> beds = {}) {
    auto cabin = std::make_unique<Cabin>("cabin", make_grid(), DOOR_POS, std::move(beds));
    auto farm = std::make_unique<Farm>("farm", make_grid(), CABIN_POS, std::move(cabin));
    auto market = std::make_unique<Interior>("market", make_grid(), DOOR_POS);
    auto community_center = std::make_unique<Interior>("community_center", make_grid(), DOOR_POS);
    auto town = std::make_unique<Town>("town", make_grid(), CABIN_POS, std::move(market),
                                       Position{1, 1}, std::move(community_center));
    std::vector<std::unique_ptr<DungeonLevel>> levels;
    for (int depth = 0; depth < Dungeon::LEVEL_COUNT; ++depth) {
        levels.push_back(std::make_unique<DungeonLevel>("level", make_grid(), depth));
    }
    return std::make_unique<World>(std::move(farm), std::move(town), Dungeon(std::move(levels)));
}

class WorldTest: public ::testing::Test {
protected:
    // Lo deja caminando al lado de la puerta de la cabaña y la usa.
    void walk_into_the_cabin() {
        player.relocate(farm(), Position{3, 4});
        player.set_direction(Direction::RIGHT);
        world->get_movement().enter_portal(1);
    }

    Farm& farm() { return world->get_farm(); }
    Cabin& cabin() { return world->get_farm().get_cabin(); }

    std::unique_ptr<World> world = make_world();
    Player& player = world->add_player(std::make_unique<Player>(
            1, Position{0, 0}, world->get_farm(), "jero", 100, 1.0f, 0, 100));
};

TEST_F(WorldTest, FarmLinksBothCabinDoors) {
    ASSERT_TRUE(farm().portal_at(CABIN_POS).has_value());
    EXPECT_EQ(farm().portal_at(CABIN_POS)->target, &cabin());
    ASSERT_TRUE(cabin().portal_at(DOOR_POS).has_value());
    EXPECT_EQ(cabin().portal_at(DOOR_POS)->target, &farm());
}

TEST_F(WorldTest, AddPlayerRegistersItInItsLocation) {
    ASSERT_EQ(farm().get_entities().size(), 1u);
    EXPECT_EQ(farm().get_entities()[0], &player);
    EXPECT_EQ(&world->location_of(1), &farm());
}

TEST_F(WorldTest, RemovePlayerUnregistersIt) {
    world->remove_player(1);
    EXPECT_TRUE(farm().get_entities().empty());
    EXPECT_TRUE(world->get_players().empty());
    world->update(1.0f);
}

TEST_F(WorldTest, UpdateMovesThePlayers) {
    player.set_direction(Direction::DOWN);
    world->update(1.0f);
    EXPECT_EQ(player.get_position(), (Position{0, 1}));
}

TEST_F(WorldTest, FarmUpdateAlsoUpdatesTheCabin) {
    walk_into_the_cabin();
    player.set_direction(Direction::UP);
    world->update(1.0f);
    EXPECT_EQ(player.get_position(), (Position{0, 3}));
}

TEST_F(WorldTest, EnteringTheDoorTravelsToTheCabin) {
    walk_into_the_cabin();
    EXPECT_EQ(&player.get_location(), &cabin());
    EXPECT_EQ(player.get_position(), DOOR_POS);
    EXPECT_TRUE(farm().get_entities().empty());
    EXPECT_EQ(cabin().get_entities().size(), 1u);
}

TEST_F(WorldTest, ArrivingThroughADoorStopsThePlayer) {
    walk_into_the_cabin();
    EXPECT_EQ(player.get_direction(), Direction::NONE);
    world->update(1.0f);
    EXPECT_EQ(&player.get_location(), &cabin());
}

TEST_F(WorldTest, UsingTheCabinDoorReturnsToTheFarm) {
    walk_into_the_cabin();
    world->get_movement().enter_portal(1);
    EXPECT_EQ(&player.get_location(), &farm());
    EXPECT_EQ(player.get_position(), (Position{CABIN_POS.x, CABIN_POS.y + 2}));
}

// --- Pelea: el jugador en (1, 1) del primer nivel, un esqueleto en (2, 1) ---

// Los esqueletos de estos tests no ven a nadie: solo reciben golpes
const std::vector<Creature*> NO_VISITORS;

class CombatTest: public WorldTest {
protected:
    CombatTest() {
        player.relocate(level(), Position{1, 1});
        auto placed = std::make_unique<Skeleton>(1000, Position{2, 1}, level(), NO_VISITORS, 0);
        skeleton = placed.get();
        level().spawn(std::move(placed));
    }

    DungeonLevel& level() { return world->get_dungeon().level(0); }
    int sword_damage() const { return server_config().player.sword_damage; }

    Skeleton* skeleton;
};

TEST_F(CombatTest, HitsTheEnemyInTheClickedCell) {
    int health = skeleton->get_health();
    EXPECT_TRUE(world->attack(player, Position{2, 1}));
    EXPECT_EQ(skeleton->get_health(), health - sword_damage());
}

TEST_F(CombatTest, AClickNextToAnEnemyBetweenCellsStillHitsIt) {
    // Está entre (2, 1) y (2, 2): el click en (2, 2) lo encuentra igual
    skeleton->place_at(Point{2.5f, 2.0f});
    int health = skeleton->get_health();
    EXPECT_TRUE(world->attack(player, Position{2, 2}));
    EXPECT_EQ(skeleton->get_health(), health - sword_damage());
}

TEST_F(CombatTest, ACellThatIsNotNextToThePlayerIsOutOfReach) {
    skeleton->place_at(Point{4.5f, 1.5f});
    EXPECT_FALSE(world->attack(player, Position{4, 1}));
}

TEST_F(CombatTest, OutsideTheDungeonThereIsNothingToHit) {
    player.relocate(farm(), Position{1, 1});
    EXPECT_FALSE(world->attack(player, Position{2, 1}));
}

TEST_F(CombatTest, WaitsBetweenHits) {
    int health = skeleton->get_health();
    EXPECT_TRUE(world->attack(player, Position{2, 1}));
    EXPECT_FALSE(world->attack(player, Position{2, 1}));
    player.update(server_config().player.attack_cooldown);
    EXPECT_TRUE(world->attack(player, Position{2, 1}));
    EXPECT_EQ(skeleton->get_health(), health - 2 * sword_damage());
}

TEST_F(CombatTest, AWeakPlayerHitsLess) {
    // Le queda 1 de 100: menos que weak_health
    player.take_damage(player.get_health() - 1);
    int health = skeleton->get_health();
    EXPECT_TRUE(world->attack(player, Position{2, 1}));
    int weak_damage = static_cast<int>(sword_damage() * server_config().player.weak_damage);
    EXPECT_EQ(skeleton->get_health(), health - weak_damage);
}

TEST_F(CombatTest, SnapshotsHaveTheEnemiesOfTheDungeon) {
    world->attack(player, Position{2, 1});
    Snapshot snapshot = SnapshotBuilder::build(0, *world);
    auto found = std::find_if(snapshot.entities.begin(), snapshot.entities.end(),
                              [](const EntityState& e) { return e.type == EntityType::Skeleton; });
    ASSERT_NE(found, snapshot.entities.end());
    EXPECT_EQ(found->entityId, skeleton->get_id());
    EXPECT_EQ(found->areaId, level().get_name());
    EXPECT_EQ(found->x, 2500u);
    EXPECT_EQ(found->y, 1500u);
    EXPECT_EQ(found->action, EntityAction::Hurt);
}

}  // namespace

namespace {

Cabin make_cabin_with_beds(std::vector<Position> slots) {
    return Cabin("cabin", make_grid(), DOOR_POS, std::move(slots));
}

TEST(CabinBedsTest, BedsAreGivenInTheOrderOfTheSlots) {
    Cabin cabin = make_cabin_with_beds({Position{1, 1}, Position{3, 1}});
    EXPECT_EQ(cabin.assign_bed(5), (Position{1, 1}));
    EXPECT_EQ(cabin.assign_bed(2), (Position{3, 1}));
    EXPECT_EQ(cabin.get_beds().size(), 2u);
}

TEST(CabinBedsTest, APlayerThatComesBackKeepsItsBed) {
    Cabin cabin = make_cabin_with_beds({Position{1, 1}, Position{3, 1}});
    cabin.assign_bed(5);
    cabin.assign_bed(2);
    EXPECT_EQ(cabin.assign_bed(5), (Position{1, 1}));
    EXPECT_EQ(cabin.get_beds().size(), 2u);
}

TEST(CabinBedsTest, WithoutFreeSlotsAssigningABedThrows) {
    Cabin cabin = make_cabin_with_beds({Position{1, 1}});
    cabin.assign_bed(5);
    EXPECT_THROW(cabin.assign_bed(6), std::logic_error);
}

// --- Los que se van y vuelven ---

// Mundo con una cama, sin jugadores: entran con spawn_player.
class ReturningPlayerTest: public ::testing::Test {
protected:
    std::unique_ptr<World> world = make_world({Position{2, 1}});

    Player& player() { return *world->find_player(1); }
    // Termina el día: despiertan todos
    void end_the_day() { world->update(world->get_clock().get_day_seconds() + 1); }
};

TEST_F(ReturningPlayerTest, WhoLeavesIsOffTheMapButNotForgotten) {
    world->spawn_player(1, "jero");
    world->remove_player(1);
    EXPECT_EQ(world->find_player(1), nullptr);
    EXPECT_TRUE(world->get_cabin().get_entities().empty());
}

TEST_F(ReturningPlayerTest, WhoComesBackKeepsWhatItHadAndWakesInItsBed) {
    world->spawn_player(1, "jero");
    player().add_gold(30);
    player().receive(ItemType::Egg, 3);
    int gold = player().get_gold();
    player().relocate(world->get_farm(), Position{1, 1});
    world->remove_player(1);
    world->spawn_player(1, "jero");
    ASSERT_NE(world->find_player(1), nullptr);
    EXPECT_EQ(player().get_gold(), gold);
    EXPECT_EQ(player().count(ItemType::Egg), 3);
    EXPECT_EQ(&player().get_location(), &world->get_cabin());
    EXPECT_EQ(world->get_cabin().get_entities().size(), 1u);
}

TEST_F(ReturningPlayerTest, IfTheDayPassesWhileAwayItWakesAsAfterABadNight) {
    world->spawn_player(1, "jero");
    world->remove_player(1);
    end_the_day();
    world->spawn_player(1, "jero");
    const ServerConfig::Player& stats = server_config().player;
    EXPECT_EQ(player().get_health(), static_cast<int>(stats.health * stats.bad_night_recovery));
    EXPECT_EQ(player().get_energy(), static_cast<int>(stats.energy * stats.bad_night_recovery));
}

// --- Guardar y retomar la partida ---

TEST_F(ReturningPlayerTest, TheSavedGameHasTheDayAndEveryoneWhoPlayed) {
    world->spawn_player(1, "jero");
    player().add_gold(10);
    int gold = player().get_gold();
    world->remove_player(1);
    end_the_day();
    SavedGame saved = world->saved();
    EXPECT_EQ(saved.day, 2);
    ASSERT_EQ(saved.players.count(1), 1u);
    EXPECT_EQ(saved.players.at(1).gold, gold);
}

TEST_F(ReturningPlayerTest, WhoEntersARestoredGameGetsWhatItHadSaved) {
    SavedGame saved{7, world->get_farm().saved(), {{1, SavedPlayer{999, 40, 30, {}}}}};
    world->restore(saved);
    EXPECT_EQ(world->get_clock().get_day(), 7);
    world->spawn_player(1, "jero");
    EXPECT_EQ(player().get_gold(), 999);
    EXPECT_EQ(player().get_health(), 40);
    EXPECT_EQ(player().get_energy(), 30);
}

TEST_F(ReturningPlayerTest, WhoDidNotEnterKeepsWhatItHadSaved) {
    SavedPlayer absent{500, 100, 100, {}};
    world->restore(SavedGame{3, world->get_farm().saved(), {{2, absent}}});
    world->spawn_player(1, "jero");
    SavedGame saved = world->saved();
    EXPECT_EQ(saved.players.at(2), absent);
    EXPECT_EQ(saved.players.count(1), 1u);
}

TEST_F(ReturningPlayerTest, UpdateTellsWhenANewDayStarts) {
    EXPECT_FALSE(world->update(1.0f));
    EXPECT_TRUE(world->update(world->get_clock().get_day_seconds()));
    EXPECT_EQ(world->get_clock().get_day(), 2);
}

}  // namespace
