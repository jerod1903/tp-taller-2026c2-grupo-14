#include "client/menu/components/CharacterPreview.h"

#include <QColor>
#include <QPainter>

#include "client/menu/styles/CharacterColors.h"

CharacterPreview::CharacterPreview(int size, QWidget* parent): QWidget(parent) {
    // Sin esto hereda el fondo global de QWidget y se ve un recuadro alrededor.
    setObjectName("characterPreview");
    setFixedSize(size, size);
}

void CharacterPreview::setCharacter(const CharacterAppearance& newAppearance,
                                    const QString& newName) {
    appearance = newAppearance;
    name = newName;
    update();  // pide un repintado
}

void CharacterPreview::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto colorIndex = appearance.hairColor % CharacterColors::PALETTE.size();
    const QColor fill(CharacterColors::PALETTE[colorIndex]);
    painter.setPen(QPen(QColor("#3E2109"), 3));
    painter.setBrush(fill);
    painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);

    // Inicial en negro o blanco segun lo claro que sea el fondo, para que se lea.
    painter.setPen(fill.lightness() > 140 ? QColor("#1E120A") : QColor("#FFF3CD"));
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(height() / 2);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, name.left(1).toUpper());
}
