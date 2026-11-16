#include "server/game/world/world.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "server/game/characters/player.h"
#include "server/game/world/dungeon/dungeon.h"
#include "server/game/world/farm/farm.h"
#include "server/game/world/town/town.h"
#include "tests/world_helpers.h"

namespace {

const Position CABIN_POS{4, 4};  // Puerta de la cabaña, en la granja.
const Position DOOR_POS{0, 4};   // Puerta de salida, dentro de la cabaña.

std::unique_ptr<World> make_world() {
    auto cabin = std::make_unique<Cabin>("cabin", make_grid(), DOOR_POS);
    auto farm = std::make_unique<Farm>("farm", make_grid(), CABIN_POS, std::move(cabin));
    auto town = std::make_unique<Town>("town", make_grid());
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

}  // namespace
