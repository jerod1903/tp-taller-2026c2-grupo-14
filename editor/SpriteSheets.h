#pragma once

#include <QPixmap>
#include <map>
#include <optional>
#include <string>

#include "common/map/MapDefinition.h"

// Recorta los dibujos de tiles y elementos segun la tabla de common/map/MapSprites.h. Cada imagen
// se lee del disco una sola vez.
class SpriteSheets {
public:
    QPixmap tile(stardew::maps::TileType type);
    // Vacio para los elementos que no tienen dibujo (donde arrancan los animales, puertas)
    std::optional<QPixmap> element(stardew::maps::ElementType type);

private:
    const QPixmap& sheet(const char* path);

    std::map<std::string, QPixmap> sheets;
};
