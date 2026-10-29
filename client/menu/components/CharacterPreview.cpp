#include "client/menu/components/CharacterPreview.h"

#include <QColor>
#include <QPainter>
#include <algorithm>

#include "client/menu/styles/CharacterColors.h"

CharacterPreview::CharacterPreview(const QSize& size, QWidget* parent):
        QWidget(parent), sprites(TALLER_ASSETS_DIR) {
    // Sin esto hereda el fondo global de QWidget y se ve un recuadro alrededor.
    setObjectName("characterPreview");
    setFixedSize(size);
}

void CharacterPreview::setCharacter(const CharacterAppearance& newAppearance,
                                    const QString& newName) {
    appearance = newAppearance;
    name = newName;
    update();  // pide un repintado
}

void CharacterPreview::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    if (!sprites.isLoaded()) {
        paintInitial(painter);
        return;
    }

    // Se agranda un numero entero de veces y sin suavizar, para que el pixel art no se vea borroso.
    const QSize canvas = CharacterSprites::canvasSize();
    const int scale = std::max(1, std::min(width() / canvas.width(), height() / canvas.height()));
    const QSize scaled = canvas * scale;
    const QRect target((width() - scaled.width()) / 2, (height() - scaled.height()) / 2,
                       scaled.width(), scaled.height());
    painter.drawImage(target, sprites.render(appearance));
}

void CharacterPreview::paintInitial(QPainter& painter) {
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
