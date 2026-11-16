#include "editor/SpriteSheets.h"

#include <QPainter>
#include <QString>
#include <QTransform>

#include "common/map/MapSprites.h"

namespace maps = stardew::maps;

QPixmap SpriteSheets::tile(maps::TileType type) {
    const maps::TileSprite& sprite = maps::TILE_SPRITES.at(type);
    QPixmap tile(maps::TILE_SIZE, maps::TILE_SIZE);
    tile.fill(Qt::black);
    QPainter painter(&tile);
    if (sprite.under) {
        painter.drawPixmap(0, 0, this->tile(*sprite.under));
    }
    QPixmap piece = sheet(sprite.sheet)
                            .copy(sprite.column * maps::TILE_SIZE + sprite.offsetX,
                                  sprite.row * maps::TILE_SIZE, maps::TILE_SIZE, sprite.height);
    painter.drawPixmap(0, 0, sprite.flipped ? piece.transformed(QTransform().scale(-1, 1)) : piece);
    return tile;
}

std::optional<QPixmap> SpriteSheets::element(maps::ElementType type) {
    auto sprite = maps::ELEMENT_SPRITES.find(type);
    if (sprite == maps::ELEMENT_SPRITES.end()) {
        return std::nullopt;
    }
    const maps::SpriteRect& rect = sprite->second.rect;
    QPixmap piece = sheet(sprite->second.sheet).copy(rect.x, rect.y, rect.width, rect.height);
    return sprite->second.flipped ? piece.transformed(QTransform().scale(-1, 1)) : piece;
}

const QPixmap& SpriteSheets::sheet(const char* path) {
    auto loaded = sheets.find(path);
    if (loaded == sheets.end()) {
        loaded = sheets.emplace(path, QPixmap(QString(TALLER_ASSETS_DIR "/") + path)).first;
    }
    return loaded->second;
}
