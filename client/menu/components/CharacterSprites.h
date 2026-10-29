#pragma once

#include <QImage>
#include <QString>

#include "common/lobby_dto.h"

// Arma la imagen del personaje de frente, quieto, superponiendo las capas: cuerpo,
// pantalon, camisa, brazos, pelo y sombrero. Las hojas se leen de disco una sola vez.
//
// Provisorio: usa las hojas de Stardew Valley, que no se pueden subir al repo (estan solo
// en el Assets/ local). Para cambiar de origen alcanza con cambiar las rutas y el formato
// de las hojas en el .cpp; el resto del menu solo ve render().
class CharacterSprites {
public:
    explicit CharacterSprites(const QString& assetsDir);

    // False si falta alguna hoja: en ese caso hay que dibujar otra cosa.
    bool isLoaded() const;

    // Tamano de la imagen que devuelve render(), en pixeles del sprite.
    static QSize canvasSize();

    QImage render(const CharacterAppearance& appearance) const;

private:
    QImage body;
    QImage hair;
    QImage shirts;
    QImage pants;
    QImage hats;
};
