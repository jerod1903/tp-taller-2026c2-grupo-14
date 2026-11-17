#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "common/logger.h"
#include "common/map/persistence/MapRepository.h"
#include "common/map/service/MapService.h"
#include "editor/Guidelines.h"
#include "editor/MapEditing.h"
#include "editor/NewMap.h"
#include "server/game/world/world_loader.h"
#include "tests/test_worlds.h"

namespace {

namespace fs = std::filesystem;
namespace maps = stardew::maps;

// En la plantilla el pueblo empieza en la columna 30
constexpr int VILLAGE_X = 30;

class NewMapTest: public ::testing::Test {
protected:
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        fs::remove_all(dir);
        fs::create_directories(dir);
    }

    void TearDown() override {
        fs::remove_all(dir);
        Logger::set_level(LogLevel::Info);
    }

    fs::path dir = fs::temp_directory_path() / "taller_new_map_test";
    maps::TomlMapRepository repository;
    maps::MapService service{repository, example_requirements()};
    MapEditing editing{example_requirements()};
};

}  // namespace

TEST_F(NewMapTest, TheTemplateHasAFarmAndAVillageJoinedByAPassage) {
    const maps::MapDefinition map = newMap("Mi valle", 1);

    ASSERT_EQ(map.areas.size(), 2U);
    EXPECT_EQ(map.areas[0].type, maps::AreaType::Farm);
    EXPECT_EQ(map.areas[1].type, maps::AreaType::Village);
    EXPECT_EQ(map.areas[1].offsetX, VILLAGE_X);
    // Un porton de cada lado y una transicion en cada sentido
    EXPECT_EQ(map.elements.size(), 2U);
    EXPECT_EQ(map.transitions.size(), 2U);
}

TEST_F(NewMapTest, TheGuidelinesComeFromTheRules) {
    const std::string text = mapGuidelines(example_requirements());

    EXPECT_NE(text.find("1 granja, 1 pueblo y una mina de 10 pisos"), std::string::npos);
    EXPECT_NE(text.find("La granja necesita la cabaña"), std::string::npos);
    EXPECT_NE(text.find("el mercado, el centro comunitario y la entrada a la mina"),
              std::string::npos);
}

TEST_F(NewMapTest, ANewMapCompletedInTheEditorIsOfferedByTheServer) {
    editing.load(newMap("Mi valle", 1));
    // Lo obligatorio: la cabana en la granja; el mercado, el centro comunitario y la entrada (que
    // crea la mina) en el pueblo. Los comerciantes estan adentro del mercado
    ASSERT_TRUE(editing.placeElement(maps::ElementType::Cabin, {2, 18}));
    ASSERT_TRUE(editing.placeElement(maps::ElementType::Market, {VILLAGE_X + 1, 8}));
    ASSERT_TRUE(editing.placeElement(maps::ElementType::CommunityCenter, {VILLAGE_X + 14, 9}));
    ASSERT_TRUE(editing.placeEntrance({VILLAGE_X + 24, 2}, AreaSize{20, 15}));

    const std::vector<maps::MapValidationIssue> issues =
            service.saveMap((dir / "mi-valle.toml").string(), editing.map());
    for (const maps::MapValidationIssue& issue: issues) {
        ADD_FAILURE() << maps::toString(issue.code);
    }

    const std::vector<GameWorld> worlds = load_worlds(dir.string(), example_requirements());
    ASSERT_EQ(worlds.size(), 1U);
    EXPECT_EQ(worlds[0].info.worldId, "mi-valle");
    EXPECT_EQ(worlds[0].info.name, "Mi valle");
}

TEST_F(NewMapTest, AMapWithErrorsIsSavedAnywayButTheServerSkipsIt) {
    // La plantilla sola no tiene mina ni edificios
    const std::vector<maps::MapValidationIssue> issues =
            service.saveMap((dir / "a-medias.toml").string(), newMap("A medias", 1));

    EXPECT_FALSE(issues.empty());
    EXPECT_TRUE(fs::exists(dir / "a-medias.toml"));
    EXPECT_TRUE(load_worlds(dir.string(), example_requirements()).empty());
}
