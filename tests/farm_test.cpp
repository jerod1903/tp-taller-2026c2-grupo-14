#include "server/game/world/farm/farm.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>

#include "tests/world_helpers.h"

namespace {

const Position CABIN_POS{4, 4};
const Position DOOR_POS{0, 4};
const Position ANIMAL_POS{1, 1};

class FarmTest: public ::testing::Test {
protected:
    Farm farm{"farm", make_grid(), DoorCells{CABIN_POS},
              std::make_unique<Cabin>("cabin", make_grid(), DOOR_POS)};

    // Cuántas entidades de la granja son animales.
    int animal_entities() const {
        return static_cast<int>(std::count_if(
                farm.get_entities().begin(), farm.get_entities().end(), [this](Entity* entity) {
                    return std::any_of(farm.get_animals().begin(), farm.get_animals().end(),
                                       [entity](const std::unique_ptr<Animal>& animal) {
                                           return animal.get() == entity;
                                       });
                }));
    }
};

}  // namespace

TEST_F(FarmTest, CountAnimalsCountsOnlyThatKind) {
    farm.add_animal(AnimalKind::Cow, ANIMAL_POS);
    farm.add_animal(AnimalKind::Chicken, ANIMAL_POS);
    farm.add_animal(AnimalKind::Chicken, ANIMAL_POS);
    EXPECT_EQ(farm.count_animals(AnimalKind::Cow), 1);
    EXPECT_EQ(farm.count_animals(AnimalKind::Chicken), 2);
}

TEST_F(FarmTest, RemoveAnimalsTakesOnlyThatKind) {
    farm.add_animal(AnimalKind::Cow, ANIMAL_POS);
    farm.add_animal(AnimalKind::Chicken, ANIMAL_POS);
    farm.add_animal(AnimalKind::Cow, ANIMAL_POS);
    farm.remove_animals(AnimalKind::Cow, 2);
    EXPECT_EQ(farm.count_animals(AnimalKind::Cow), 0);
    EXPECT_EQ(farm.count_animals(AnimalKind::Chicken), 1);
}

TEST_F(FarmTest, RemovedAnimalsStopBeingEntitiesOfTheFarm) {
    farm.add_animal(AnimalKind::Cow, ANIMAL_POS);
    farm.add_animal(AnimalKind::Cow, ANIMAL_POS);
    farm.remove_animals(AnimalKind::Cow, 1);
    EXPECT_EQ(farm.get_animals().size(), 1u);
    EXPECT_EQ(animal_entities(), 1);
}

TEST_F(FarmTest, RemovingMoreThanThereAreThrowsAndRemovesNothing) {
    farm.add_animal(AnimalKind::Cow, ANIMAL_POS);
    EXPECT_THROW(farm.remove_animals(AnimalKind::Cow, 2), std::invalid_argument);
    EXPECT_EQ(farm.count_animals(AnimalKind::Cow), 1);
}
