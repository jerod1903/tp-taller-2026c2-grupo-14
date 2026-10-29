#pragma once

#include <QString>
#include <QWidget>

#include "client/menu/components/CharacterSprites.h"
#include "common/lobby_dto.h"

class QPainter;
class QPaintEvent;

// Imagen del personaje a partir de su apariencia. Si no estan las hojas de sprites, dibuja
// un recuadro con la inicial del nombre.
class CharacterPreview: public QWidget {
public:
    explicit CharacterPreview(int size, QWidget* parent = nullptr);

    void setCharacter(const CharacterAppearance& appearance, const QString& name);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void paintInitial(QPainter& painter);

    CharacterSprites sprites;
    CharacterAppearance appearance{};
    QString name;
};
