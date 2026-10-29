#include "client/menu/components/OptionStepper.h"

#include <QHBoxLayout>
#include <QLabel>

#include "client/menu/components/MenuButton.h"

namespace {
constexpr int ARROW_WIDTH = 48;
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

    auto* previous = new MenuButton("‹", this);
    auto* next = new MenuButton("›", this);
    previous->setFixedWidth(ARROW_WIDTH);
    next->setFixedWidth(ARROW_WIDTH);
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
        // Se muestra desde 1 para que se lea natural ("1 / 10" y no "0 / 10").
        valueLabel->setText(QString("%1  %2 / %3").arg(title).arg(current + 1).arg(count));
    }
}
