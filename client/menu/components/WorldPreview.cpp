#include "client/menu/components/WorldPreview.h"

#include <exception>

#include "client/mapa-general/grilla_mapa.h"
#include "client/mapa-general/mundo.h"

std::optional<QImage> worldPreviewImage(const std::string& worldId) {
    GrillaMapa grilla;
    try {
        grilla = grilla_del_mapa(cargar_mundo(worldId));
    } catch (const std::exception&) {
        return std::nullopt;
    }
    if (grilla.ancho == 0 || grilla.alto == 0) {
        return std::nullopt;
    }
    QImage image(grilla.ancho, grilla.alto, QImage::Format_RGB32);
    for (int y = 0; y < grilla.alto; ++y) {
        for (int x = 0; x < grilla.ancho; ++x) {
            const ColorMapa& color = grilla.color(x, y);
            image.setPixel(x, y, qRgb(color.r, color.g, color.b));
        }
    }
    return image;
}
