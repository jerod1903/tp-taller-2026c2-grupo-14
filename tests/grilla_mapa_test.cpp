#include "client/mapa-general/grilla_mapa.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "client/mapa-general/colores_mapa.h"
#include "client/mapa-general/mundo.h"

namespace {

namespace maps = stardew::maps;
using maps::AreaType;
using maps::ElementLayer;
using maps::ElementType;
using maps::TileType;

maps::MapArea area(const std::string& id, AreaType type, int ancho, int alto, int x, int y,
                   TileType relleno) {
    maps::MapArea area{
            id,
            type,
            ancho,
            alto,
            std::nullopt,
            std::vector<std::vector<TileType>>(alto, std::vector<TileType>(ancho, relleno))};
    area.offsetX = x;
    area.offsetY = y;
    return area;
}

// Granja de 3x2 en (10, 5) y pueblo de 2x2 en (13, 5), con un hueco abajo del pueblo; un piso de
// la mina de 2x1 en (10, 8) y un interior que no es del mundo
maps::MapDefinition mapa() {
    maps::MapDefinition mapa{1, "Test", {}, {}, {}};
    mapa.areas.push_back(area("granja", AreaType::Farm, 3, 2, 10, 5, TileType::Grass));
    mapa.areas.push_back(area("pueblo", AreaType::Village, 2, 2, 13, 5, TileType::Path));
    mapa.areas.push_back(area("mina-1", AreaType::DungeonLevel, 2, 1, 10, 8, TileType::CaveFloor));
    mapa.areas.push_back(area("cabana", AreaType::CabinArea, 4, 4, 0, 0, TileType::HouseFloor));
    mapa.areas[0].tiles[1][2] = TileType::Water;
    return mapa;
}

ColorMapa terreno(TileType tipo) { return COLORES_TERRENO.at(tipo); }

}  // namespace

TEST(GrillaMapaTest, ArrancaEnLaPrimeraAreaYNoIncluyeLosInteriores) {
    GrillaMapa grilla = grilla_del_mapa(mapa());
    EXPECT_EQ(grilla.ancho, 5);
    EXPECT_EQ(grilla.alto, 4);
    EXPECT_EQ(grilla.areas.at("granja"), (LugarArea{0, 0, 3, 2}));
    EXPECT_EQ(grilla.areas.at("pueblo"), (LugarArea{3, 0, 2, 2}));
    EXPECT_EQ(grilla.areas.at("mina-1"), (LugarArea{0, 3, 2, 1}));
    EXPECT_FALSE(grilla.areas.contains("cabana"));
}

TEST(GrillaMapaTest, CadaCasillaTieneElColorDeSuTerreno) {
    GrillaMapa grilla = grilla_del_mapa(mapa());
    EXPECT_EQ(grilla.color(0, 0), terreno(TileType::Grass));
    EXPECT_EQ(grilla.color(2, 1), terreno(TileType::Water));
    EXPECT_EQ(grilla.color(4, 1), terreno(TileType::Path));
    EXPECT_EQ(grilla.color(1, 3), terreno(TileType::CaveFloor));
}

TEST(GrillaMapaTest, LoQueNoEsDeNingunAreaEsFondo) {
    GrillaMapa grilla = grilla_del_mapa(mapa());
    EXPECT_EQ(grilla.color(0, 2), COLOR_FONDO);
    EXPECT_EQ(grilla.color(4, 3), COLOR_FONDO);
}

TEST(GrillaMapaTest, LosElementosTapanSuBaseConSuColor) {
    maps::MapDefinition definicion = mapa();
    definicion.elements.push_back(
            {"arbol", "granja", ElementType::Tree, ElementLayer::Structure, 1, 1, 2, 1});
    GrillaMapa grilla = grilla_del_mapa(definicion);
    EXPECT_EQ(grilla.color(1, 1), COLORES_ELEMENTO.at(ElementType::Tree));
    EXPECT_EQ(grilla.color(2, 1), COLORES_ELEMENTO.at(ElementType::Tree));
    EXPECT_EQ(grilla.color(1, 0), terreno(TileType::Grass));
}

TEST(GrillaMapaTest, LosElementosSinColorNoSeDibujan) {
    maps::MapDefinition definicion = mapa();
    definicion.elements.push_back(
            {"animales", "granja", ElementType::AnimalSpawn, ElementLayer::Object, 0, 0, 1, 1});
    GrillaMapa grilla = grilla_del_mapa(definicion);
    EXPECT_EQ(grilla.color(0, 0), terreno(TileType::Grass));
}

TEST(GrillaMapaTest, UnMapaSinAreasDaUnaGrillaVacia) {
    GrillaMapa grilla = grilla_del_mapa(maps::MapDefinition{1, "Vacio", {}, {}, {}});
    EXPECT_EQ(grilla.ancho, 0);
    EXPECT_EQ(grilla.alto, 0);
    EXPECT_TRUE(grilla.colores.empty());
}

TEST(CargarMundoTest, LeeElMapaDelMundoPorSuId) {
    GrillaMapa grilla = grilla_del_mapa(cargar_mundo("valle"));
    EXPECT_TRUE(grilla.areas.contains("granja"));
    EXPECT_TRUE(grilla.areas.contains("pueblo"));
}

TEST(CargarMundoTest, UnIdQueSaleDeLaCarpetaSeRechaza) {
    EXPECT_THROW(cargar_mundo("../maps/valle"), std::runtime_error);
    EXPECT_THROW(cargar_mundo(""), std::runtime_error);
}
