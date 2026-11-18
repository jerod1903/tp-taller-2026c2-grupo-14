#include "server/game/commands/interact_command.h"

#include <memory>
#include <variant>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "server/game/characters/animals/chicken/chicken.h"
#include "server/game/characters/animals/cow/cow.h"
#include "tests/fake_game_context.h"

namespace {

// El jugador del FakeGameContext esta en la celda (0, 0): su centro es (0.5, 0.5).
class InteractCommandTest: public ::testing::Test {
protected:
    // Pone un animal en `cell`; si `ready`, ya tiene leche (sin moverse de esa celda).
    Animal& add(AnimalKind kind, const Position& cell, bool ready = true) {
        if (kind == AnimalKind::Cow) {
            context.animals.push_back(std::make_unique<Cow>(next_id++, cell, context.location));
        } else {
            context.animals.push_back(std::make_unique<Chicken>(next_id++, cell, context.location));
        }
        Animal& animal = *context.animals.back();
        if (ready) {
            for (int day = 0; day < server_config().animals.cow.production_days; ++day) {
                animal.start_new_day();
            }
        }
        return animal;
    }

    void interact() { InteractCommand(FakeGameContext::PLAYER_ID).execute(context); }

    int milk_in_inventory() {
        int total = 0;
        for (const InventorySlot& slot: context.player.get_inventory_state().slots) {
            total += slot.item == ItemType::Milk ? slot.quantity : 0;
        }
        return total;
    }

    FakeGameContext context;
    uint32_t next_id = 0;
};

}  // namespace

TEST_F(InteractCommandTest, MilksAReadyCowNextToThePlayer) {
    Animal& cow = add(AnimalKind::Cow, Position{1, 0});
    interact();

    EXPECT_FALSE(cow.is_ready());
    EXPECT_EQ(milk_in_inventory(), 1);
    ASSERT_EQ(context.sent.size(), 1u);
    EXPECT_EQ(context.sent[0].first, FakeGameContext::PLAYER_ID);
    EXPECT_TRUE(std::holds_alternative<InventoryState>(context.sent[0].second));
}

TEST_F(InteractCommandTest, ReachesACowInTheDiagonalCell) {
    Animal& cow = add(AnimalKind::Cow, Position{1, 1});  // a 1,41 casillas
    interact();
    EXPECT_FALSE(cow.is_ready());
}

TEST_F(InteractCommandTest, ACowTooFarIsNotMilked) {
    Animal& cow = add(AnimalKind::Cow, Position{2, 0});  // a 2 casillas
    interact();

    EXPECT_TRUE(cow.is_ready());
    EXPECT_EQ(milk_in_inventory(), 0);
    EXPECT_TRUE(context.sent.empty());
}

TEST_F(InteractCommandTest, MilksTheNearestOfTwo) {
    Animal& far = add(AnimalKind::Cow, Position{1, 1});
    Animal& near = add(AnimalKind::Cow, Position{1, 0});
    interact();

    EXPECT_FALSE(near.is_ready());
    EXPECT_TRUE(far.is_ready());
}

TEST_F(InteractCommandTest, ACowThatIsNotReadyIsIgnored) {
    add(AnimalKind::Cow, Position{1, 0}, false);
    interact();
    EXPECT_EQ(milk_in_inventory(), 0);
    EXPECT_TRUE(context.sent.empty());
}

TEST_F(InteractCommandTest, ChickensAreNotMilked) {
    add(AnimalKind::Chicken, Position{1, 0});
    interact();
    EXPECT_EQ(milk_in_inventory(), 0);
}

TEST_F(InteractCommandTest, ACowInAnotherAreaIsIgnored) {
    TestLocation other;
    context.animals.push_back(std::make_unique<Cow>(0, Position{0, 0}, other));
    for (int day = 0; day < server_config().animals.cow.production_days; ++day) {
        context.animals.back()->start_new_day();
    }
    interact();
    EXPECT_TRUE(context.animals.back()->is_ready());
}

TEST_F(InteractCommandTest, WithAFullInventoryTheCowStaysReadyAndThePlayerIsWarned) {
    context.player.receive(ItemType::Milk, 250);  // llena los 5 slots libres
    Animal& cow = add(AnimalKind::Cow, Position{1, 0});
    interact();

    EXPECT_TRUE(cow.is_ready());
    ASSERT_EQ(context.sent.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<ChatMessage>(context.sent[0].second));
    EXPECT_EQ(std::get<ChatMessage>(context.sent[0].second).kind, ChatKind::System);
}
