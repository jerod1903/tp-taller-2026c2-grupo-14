#include "server/persistence/saved_game.h"

#include <memory>
#include <utility>

#include <gtest/gtest.h>

#include "server/game/characters/player.h"
#include "server/game/world/farm/crop.h"
#include "server/game/world/farm/farm.h"
#include "tests/world_helpers.h"

// Cada parte del modelo se guarda y se restaura tal como estaba: lo que se guarda de la copia
// restaurada es lo mismo que se guardó.

namespace {

const Position CABIN_POS{4, 4};
const Position DOOR_POS{0, 4};

std::unique_ptr<Farm> make_farm() {
    auto cabin = std::make_unique<Cabin>("cabin", make_grid(), DOOR_POS);
    return std::make_unique<Farm>("farm", make_grid(10, 10), DoorCells{CABIN_POS},
                                  std::move(cabin));
}

TEST(SavedGameTest, ACropIsRestoredAsItWas) {
    Crop crop(CropType::Potato, 30.0f);
    crop.water(60.0f);
    crop.update_time(700.0f);
    crop.grow();
    SavedCrop saved = crop.saved();
    EXPECT_EQ(saved.type, CropType::Potato);
    EXPECT_EQ(Crop(saved).saved(), saved);
}

TEST(SavedGameTest, APlayerIsRestoredWithItsGoldLifeEnergyAndInventory) {
    TestLocation location;
    Player player(1, Position{0, 0}, location, "jero", 100, 1.0f, 50, 100);
    player.add_gold(25);
    player.take_damage(30);
    player.spend_energy(10);
    player.receive(ItemType::Egg, 4, ItemQuality::Good);
    player.select_inventory_slot(4);
    SavedPlayer saved = player.saved();
    EXPECT_EQ(saved.gold, 75);
    EXPECT_EQ(saved.health, 70);
    EXPECT_EQ(saved.energy, 90);

    Player copy(2, Position{0, 0}, location, "otro", 100, 1.0f, 0, 100);
    copy.restore(saved);
    EXPECT_EQ(copy.saved(), saved);
    EXPECT_EQ(copy.count(ItemType::Egg), 4);
    EXPECT_EQ(copy.selected_item(), ItemType::Egg);
}

TEST(SavedGameTest, AFarmIsRestoredWithItsPlotsGroundAndAnimals) {
    std::unique_ptr<Farm> farm = make_farm();
    // En el pasto, la primera vez queda tierra y la segunda, arada
    ASSERT_TRUE(farm->dig(Position{1, 1}));
    ASSERT_TRUE(farm->dig(Position{1, 1}));
    ASSERT_TRUE(farm->plant(Position{1, 1}, CropType::Strawberry));
    ASSERT_TRUE(farm->dig(Position{2, 1}));
    ASSERT_TRUE(farm->drop(Position{5, 5}, ItemType::Egg));
    ASSERT_TRUE(farm->add_animal(AnimalKind::Cow, Position{6, 6}));
    ASSERT_TRUE(farm->add_animal(AnimalKind::Chicken, Position{7, 7}));
    farm->start_new_day();
    SavedFarm saved = farm->saved();
    EXPECT_EQ(saved.plots.size(), 2u);
    EXPECT_EQ(saved.animals.size(), 2u);

    std::unique_ptr<Farm> copy = make_farm();
    ASSERT_TRUE(copy->add_animal(AnimalKind::Cow, Position{3, 3}));
    copy->restore(saved);
    EXPECT_EQ(copy->saved(), saved);
    // Los animales que tenía se reemplazan: quedan los dos guardados, registrados en la granja
    EXPECT_EQ(copy->get_animals().size(), 2u);
    EXPECT_EQ(copy->get_entities().size(), 2u);
}

}  // namespace
