#include "common/map/InteriorWalls.h"

#include <vector>

namespace stardew::maps {
namespace {

using Tiles = std::vector<std::vector<TileType>>;

// De la fila de arriba del piso hacia arriba
constexpr TileType BACK_WALL[BACK_WALL_ROWS] = {TileType::WallBase, TileType::Wall,
                                                TileType::WallUpper, TileType::WallTop};

class Walls {
public:
    explicit Walls(const MapArea& area): floor(area.tiles), tiles(area.tiles) {}

    Tiles build() {
        forEachWall([this](int x, int y) { tiles[y][x] = backWall(x, y); });
        forEachWall([this](int x, int y) {
            if (tiles[y][x] == TileType::Void) {
                tiles[y][x] = side(x, y);
            }
        });
        forEachWall([this](int x, int y) {
            if (tiles[y][x] == TileType::Void && isFloor(x, y - 1)) {
                tiles[y][x] = TileType::WallBottom;
            }
        });
        forEachWall([this](int x, int y) {
            if (tiles[y][x] == TileType::Void) {
                tiles[y][x] = corner(x, y);
            }
        });
        return tiles;
    }

private:
    template <typename Visit>
    void forEachWall(Visit visit) {
        for (int y = 0; y < static_cast<int>(floor.size()); ++y) {
            for (int x = 0; x < static_cast<int>(floor[y].size()); ++x) {
                if (!isFloor(x, y)) {
                    visit(x, y);
                }
            }
        }
    }

    bool inside(int x, int y) const {
        return y >= 0 && y < static_cast<int>(floor.size()) && x >= 0 &&
               x < static_cast<int>(floor[y].size());
    }

    bool isFloor(int x, int y) const { return inside(x, y) && isWalkableTerrain(floor[y][x]); }

    // Un pasillo de entrada: piso de una casilla de ancho que sale del cuarto hacia abajo
    bool isCorridor(int x, int y) const {
        return isFloor(x, y) && isFloor(x, y - 1) && !isFloor(x - 1, y) && !isFloor(x + 1, y);
    }

    // El hueco que une dos cuartos: piso de una casilla de alto entre dos pisos
    bool isPassage(int x, int y) const {
        return isFloor(x, y) && isFloor(x - 1, y) && isFloor(x + 1, y) && !isFloor(x, y - 1) &&
               !isFloor(x, y + 1);
    }

    TileType at(int x, int y) const { return inside(x, y) ? tiles[y][x] : TileType::Void; }

    // Lo que queda al lado de un piso es costado, aunque tambien tenga piso abajo (como arriba del
    // hueco que une dos cuartos)
    TileType backWall(int x, int y) const {
        if (isFloor(x - 1, y) || isFloor(x + 1, y)) {
            return TileType::Void;
        }
        for (int row = 0; row < BACK_WALL_ROWS; ++row) {
            if (isPassage(x, y + row + 1)) {
                return TileType::Void;
            }
            if (isFloor(x, y + row + 1)) {
                return BACK_WALL[row];
            }
        }
        return TileType::Void;
    }

    bool isBackWall(int x, int y) const {
        TileType tile = at(x, y);
        return tile == TileType::WallUpper || tile == TileType::Wall || tile == TileType::WallBase;
    }

    // La moldura de los costados acompaña al piso y a la pared del fondo, salvo al borde de arriba.
    // Al lado de un pasillo, en la fila del zocalo del cuarto, el zocalo dobla hacia el pasillo
    bool isRoom(int x, int y) const { return isFloor(x, y) || isBackWall(x, y); }

    TileType side(int x, int y) const {
        // Entre dos cuartos (o un cuarto y la pared del fondo del otro) va una sola pared
        if (isRoom(x - 1, y) && isRoom(x + 1, y)) {
            return TileType::WallDivider;
        }
        const bool bottomRow = isFloor(x, y - 1);
        if (isFloor(x + 1, y)) {
            return isCorridor(x + 1, y) && bottomRow ? TileType::CorridorLeft : TileType::WallLeft;
        }
        if (isFloor(x - 1, y)) {
            return isCorridor(x - 1, y) && bottomRow ? TileType::CorridorRight :
                                                       TileType::WallRight;
        }
        if (isBackWall(x + 1, y)) {
            return TileType::WallLeft;
        }
        if (isBackWall(x - 1, y)) {
            return TileType::WallRight;
        }
        return TileType::Void;
    }

    // El zocalo sigue de largo por debajo de la pared que separa dos cuartos
    TileType corner(int x, int y) const {
        if (at(x - 1, y) == TileType::WallBottom && at(x + 1, y) == TileType::WallBottom) {
            return TileType::WallBottom;
        }
        if (at(x, y - 1) == TileType::WallLeft && at(x + 1, y) == TileType::WallBottom) {
            return TileType::WallBottomLeft;
        }
        if (at(x, y - 1) == TileType::WallRight && at(x - 1, y) == TileType::WallBottom) {
            return TileType::WallBottomRight;
        }
        return TileType::Void;
    }

    const Tiles& floor;
    Tiles tiles;
};

}  // namespace

void addWalls(MapArea& area) { area.tiles = Walls(area).build(); }

}  // namespace stardew::maps
