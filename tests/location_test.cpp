#include "server/game/world/location.h"

#include <optional>

#include <gtest/gtest.h>

#include "tests/world_helpers.h"

namespace {

// Al actualizarse se muda a otra Location: prueba que update_entities tolera bajas.
class Wanderer: public TestCreature {
public:
    Wanderer(Location& from, Location& to): TestCreature(from), to(to) {}
    void update(float dt) override {
        TestCreature::update(dt);
        relocate(to, Position{0, 0});
    }

private:
    Location& to;
};

class LocationTest: public ::testing::Test {
protected:
    TestLocation location{make_grid(5, 5, {Position{1, 1}})};
    TestLocation other;
};

TEST_F(LocationTest, WalkableTileInsideTheGridIsWalkable) {
    EXPECT_TRUE(location.is_walkable(Position{0, 0}));
}

TEST_F(LocationTest, WallIsNotWalkable) { EXPECT_FALSE(location.is_walkable(Position{1, 1})); }

TEST_F(LocationTest, OutOfBoundsIsNotWalkable) {
    EXPECT_FALSE(location.is_walkable(Position{-1, 0}));
    EXPECT_FALSE(location.is_walkable(Position{5, 0}));
}

TEST_F(LocationTest, PortalAtFindsThePortalByOrigin) {
    location.add_portal(Portal{Position{4, 4}, &other, Point{2.5f, 2.5f}});
    std::optional<Portal> portal = location.portal_at(Position{4, 4});
    ASSERT_TRUE(portal.has_value());
    EXPECT_EQ(portal->target, &other);
    EXPECT_EQ(cell_of(portal->arrival), (Position{2, 2}));
}

TEST_F(LocationTest, PortalAtWithoutPortalIsEmpty) {
    location.add_portal(Portal{Position{4, 4}, &other, Point{2.5f, 2.5f}});
    EXPECT_FALSE(location.portal_at(Position{2, 2}).has_value());
}

TEST_F(LocationTest, AddAndRemoveEntities) {
    TestCreature first(location);
    TestCreature second(location);
    location.add_entity(first);
    location.add_entity(second);
    location.remove_entity(first);
    ASSERT_EQ(location.get_entities().size(), 1u);
    EXPECT_EQ(location.get_entities()[0], &second);
}

TEST_F(LocationTest, RemovingAnUnknownEntityDoesNothing) {
    TestCreature registered(location);
    TestCreature stranger(location);
    location.add_entity(registered);
    location.remove_entity(stranger);
    EXPECT_EQ(location.get_entities().size(), 1u);
}

TEST_F(LocationTest, UpdateUpdatesEveryEntity) {
    TestCreature first(location);
    TestCreature second(location);
    location.add_entity(first);
    location.add_entity(second);
    location.update(1.0f);
    EXPECT_EQ(first.updates, 1);
    EXPECT_EQ(second.updates, 1);
}

TEST_F(LocationTest, AnEntityCanLeaveDuringTheUpdate) {
    Wanderer wanderer(location, other);
    TestCreature stays(location);
    location.add_entity(wanderer);
    location.add_entity(stays);
    location.update(1.0f);
    EXPECT_EQ(stays.updates, 1);
    EXPECT_EQ(location.get_entities().size(), 1u);
    EXPECT_EQ(&wanderer.get_location(), &other);
    EXPECT_EQ(other.get_entities().size(), 1u);
}

}  // namespace
