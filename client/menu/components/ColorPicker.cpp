#include "client/menu/components/ColorPicker.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QPushButton>

#include "client/menu/styles/CharacterColors.h"

namespace {
constexpr int COLUMNS = 8;
constexpr int SWATCH_SIZE = 28;
}  // namespace

ColorPicker::ColorPicker(QWidget* parent): QWidget(parent), swatches(new QButtonGroup(this)) {
    // Sin esto hereda el fondo global de QWidget.
    setObjectName("colorPicker");
    swatches->setExclusive(true);

    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    for (int i = 0; i < static_cast<int>(CharacterColors::PALETTE.size()); ++i) {
        auto* swatch = new QPushButton(this);
        swatch->setObjectName("colorSwatch");
        swatch->setCheckable(true);
        swatch->setFixedSize(SWATCH_SIZE, SWATCH_SIZE);
        swatch->setCursor(Qt::PointingHandCursor);
        // El color de cada muestra es un dato de la paleta, no del estilo: por eso va aca.
        swatch->setStyleSheet(QString("background-color: %1;").arg(CharacterColors::PALETTE[i]));
        swatches->addButton(swatch, i);
        layout->addWidget(swatch, i / COLUMNS, i % COLUMNS);
    }
    setValue(0);

    connect(swatches, &QButtonGroup::idClicked, this, &ColorPicker::valueChanged);
}

int ColorPicker::value() const { return swatches->checkedId(); }

void ColorPicker::setValue(int index) {
    if (QAbstractButton* swatch = swatches->button(index)) {
        swatch->setChecked(true);
    }
}
