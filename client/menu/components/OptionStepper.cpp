#include "client/menu/components/OptionStepper.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

#include "client/menu/styles/StyleManager.h"

namespace {
constexpr int ARROW_WIDTH = 48;
constexpr int ARROW_ICON_WIDTH = 26;
constexpr int ARROW_ICON_HEIGHT = 30;
constexpr int ARROW_HEIGHT = 40;

// Solo el simbolo de la flecha: el objectName quita el recuadro de madera en la hoja global.
QPushButton* createArrowButton(const QString& iconPath, QWidget* parent) {
    auto* button = new QPushButton(parent);
    button->setObjectName("arrowButton");
    button->setIcon(QIcon(QPixmap(iconPath)));
    button->setIconSize(QSize(ARROW_ICON_WIDTH, ARROW_ICON_HEIGHT));
    button->setFixedSize(ARROW_WIDTH, ARROW_HEIGHT);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}
}  // namespace

OptionStepper::OptionStepper(const QString& title, int count, const QString& noneText,
                             QWidget* parent):
        QWidget(parent),
        title(title),
        count(count),
        noneText(noneText),
        valueLabel(new QLabel(this)) {
    // Sin esto hereda el fondo global de QWidget.
    setObjectName("optionStepper");

    auto* previous = createArrowButton(":/menu/botones/flecha_izq.png", this);
    auto* next = createArrowButton(":/menu/botones/flecha_der.png", this);
    valueLabel->setAlignment(Qt::AlignCenter);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(previous);
    layout->addWidget(valueLabel, 1);
    layout->addWidget(next);

    connect(previous, &QPushButton::clicked, this, [this]() { step(-1); });
    connect(next, &QPushButton::clicked, this, [this]() { step(1); });
    refreshText();
}

int OptionStepper::value() const { return current; }

void OptionStepper::setValue(int newValue) {
    current = ((newValue % count) + count) % count;
    refreshText();
}

void OptionStepper::step(int delta) {
    setValue(current + delta);
    emit valueChanged(current);
}

void OptionStepper::refreshText() {
    if (current == 0 && !noneText.isEmpty()) {
        valueLabel->setText(noneText);
    } else {
        // Se muestra desde 1 para que se lea natural ("1 / 10" y no "0 / 10"). Si el 0 es
        // "ninguno" no cuenta como opcion: el primer sombrero es "1 / 4" y no "2 / 5".
        const int offset = noneText.isEmpty() ? 1 : 0;
        valueLabel->setText(
                QString("%1  %2 / %3").arg(title).arg(current + offset).arg(count - 1 + offset));
    }
}
