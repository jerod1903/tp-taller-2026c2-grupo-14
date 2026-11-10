#include "editor/SpriteSheets.h"

#include <QString>

#include "common/map/MapSprites.h"

namespace maps = stardew::maps;

QPixmap SpriteSheets::tile(maps::TileType type) {
    const maps::TileSprite& sprite = maps::TILE_SPRITES.at(type);
    return sheet(sprite.sheet)
            .copy(sprite.column * maps::TILE_SIZE, sprite.row * maps::TILE_SIZE, maps::TILE_SIZE,
                  maps::TILE_SIZE);
}

std::optional<QPixmap> SpriteSheets::element(maps::ElementType type) {
    auto sprite = maps::ELEMENT_SPRITES.find(type);
    if (sprite == maps::ELEMENT_SPRITES.end()) {
        return std::nullopt;
    }
    const maps::SpriteRect& rect = sprite->second.rect;
    return sheet(sprite->second.sheet).copy(rect.x, rect.y, rect.width, rect.height);
}

const QPixmap& SpriteSheets::sheet(const char* path) {
    auto loaded = sheets.find(path);
    if (loaded == sheets.end()) {
        loaded = sheets.emplace(path, QPixmap(QString(TALLER_ASSETS_DIR "/") + path)).first;
    }
    return loaded->second;
}
