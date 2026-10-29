#include "client/menu/components/CharacterSprites.h"

#include <QColor>
#include <QPainter>
#include <QPoint>
#include <QRect>

#include "client/menu/styles/CharacterColors.h"

namespace {
// Formato de las hojas de Stardew Valley. Cada parte esta en una grilla de celdas y, dentro
// de la celda, la primera vista es la de frente.
const QString CHARACTERS_DIR = "/Characters/StardewValley/StardewValley_Character_";

// Cuerpo: cuadros de 16x32. El primero es el de frente quieto y sus brazos estan 6 cuadros
// a la derecha (se dibujan aparte, encima de la camisa).
constexpr int FRAME_WIDTH = 16;
constexpr int FRAME_HEIGHT = 32;
const QRect BODY_FRAME(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
const QRect ARMS_FRAME(6 * FRAME_WIDTH, 0, FRAME_WIDTH, FRAME_HEIGHT);

// Pelo: celdas de 16x96 (frente, costado, espalda), 16 por fila. Viene en gris para teñirlo.
constexpr int HAIR_CELL_HEIGHT = 96;
constexpr int HAIR_PER_ROW = 16;
const QPoint HAIR_OFFSET(0, 1);

// Camisa: celdas de 8x32 (las 4 vistas de 8x8), 16 por fila. Va sobre el pecho.
constexpr int SHIRT_SIZE = 8;
constexpr int SHIRT_CELL_HEIGHT = 32;
constexpr int SHIRTS_PER_ROW = 16;
const QPoint SHIRT_OFFSET(4, 14);

// Pantalon: un bloque de 192x688 por pantalon, 10 por fila, con los mismos cuadros que el
// cuerpo. Viene en gris: se tiñe con un color fijo porque la apariencia no elige color.
constexpr int PANTS_BLOCK_WIDTH = 192;
constexpr int PANTS_BLOCK_HEIGHT = 688;
constexpr int PANTS_PER_ROW = 10;
const QColor PANTS_COLOR(46, 85, 183);

// Sombrero: celdas de 20x80 (las 4 vistas de 20x20), 12 por fila. Sobresale de la cabeza.
constexpr int HAT_SIZE = 20;
constexpr int HAT_CELL_HEIGHT = 80;
constexpr int HATS_PER_ROW = 12;
const QPoint HAT_OFFSET(-2, -3);

// Donde va el cuadro del cuerpo dentro de la imagen final: deja lugar para el sombrero.
const QPoint FRAME_ORIGIN(4, 6);
const QSize CANVAS_SIZE(24, 40);

QImage loadSheet(const QString& assetsDir, const QString& part) {
    QImage sheet(assetsDir + CHARACTERS_DIR + part + ".png");
    return sheet.convertToFormat(QImage::Format_ARGB32);
}

// Multiplica cada pixel por el color: las hojas en gris quedan de ese color con sus sombras.
QImage tinted(const QImage& source, const QColor& color) {
    QImage result = source.copy();
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            const QColor pixel = result.pixelColor(x, y);
            result.setPixelColor(
                    x, y,
                    QColor(pixel.red() * color.red() / 255, pixel.green() * color.green() / 255,
                           pixel.blue() * color.blue() / 255, pixel.alpha()));
        }
    }
    return result;
}
}  // namespace

CharacterSprites::CharacterSprites(const QString& assetsDir):
        body(loadSheet(assetsDir, "Farmer Base")),
        hair(loadSheet(assetsDir, "Hairstyles")),
        shirts(loadSheet(assetsDir, "Shirts")),
        pants(loadSheet(assetsDir, "Pants")),
        hats(loadSheet(assetsDir, "Hats")) {}

bool CharacterSprites::isLoaded() const {
    return !body.isNull() && !hair.isNull() && !shirts.isNull() && !pants.isNull() &&
           !hats.isNull();
}

QSize CharacterSprites::canvasSize() { return CANVAS_SIZE; }

QImage CharacterSprites::render(const CharacterAppearance& appearance) const {
    QImage canvas(CANVAS_SIZE, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);

    const QRect pantsFrame((appearance.pants % PANTS_PER_ROW) * PANTS_BLOCK_WIDTH,
                           (appearance.pants / PANTS_PER_ROW) * PANTS_BLOCK_HEIGHT, FRAME_WIDTH,
                           FRAME_HEIGHT);
    const QRect shirtFrame((appearance.shirt % SHIRTS_PER_ROW) * SHIRT_SIZE,
                           (appearance.shirt / SHIRTS_PER_ROW) * SHIRT_CELL_HEIGHT, SHIRT_SIZE,
                           SHIRT_SIZE);
    const QRect hairFrame((appearance.hairStyle % HAIR_PER_ROW) * FRAME_WIDTH,
                          (appearance.hairStyle / HAIR_PER_ROW) * HAIR_CELL_HEIGHT, FRAME_WIDTH,
                          FRAME_HEIGHT);
    const auto colorIndex = appearance.hairColor % CharacterColors::PALETTE.size();
    const QColor hairColor(CharacterColors::PALETTE[colorIndex]);

    // El orden importa: cada capa tapa a las anteriores.
    painter.drawImage(FRAME_ORIGIN, body, BODY_FRAME);
    painter.drawImage(FRAME_ORIGIN, tinted(pants.copy(pantsFrame), PANTS_COLOR));
    painter.drawImage(FRAME_ORIGIN + SHIRT_OFFSET, shirts, shirtFrame);
    painter.drawImage(FRAME_ORIGIN, body, ARMS_FRAME);
    painter.drawImage(FRAME_ORIGIN + HAIR_OFFSET, tinted(hair.copy(hairFrame), hairColor));
    // El 0 es "sin sombrero", asi que el sombrero n esta en la celda n - 1.
    if (appearance.hat > 0) {
        const int hat = appearance.hat - 1;
        const QRect hatFrame((hat % HATS_PER_ROW) * HAT_SIZE,
                             (hat / HATS_PER_ROW) * HAT_CELL_HEIGHT, HAT_SIZE, HAT_SIZE);
        painter.drawImage(FRAME_ORIGIN + HAT_OFFSET, hats, hatFrame);
    }
    return canvas;
}
