#pragma once

#include <QString>
#include <QWidget>

#include "client/menu/backend/LobbyTypes.h"

class QPaintEvent;

// Imagen del personaje a partir de su apariencia.
class CharacterPreview: public QWidget {
public:
    explicit CharacterPreview(int size, QWidget* parent = nullptr);

    void setCharacter(const CharacterAppearance& appearance, const QString& name);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    CharacterAppearance appearance;
    QString name;
};
