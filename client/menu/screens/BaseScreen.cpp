#include "client/menu/screens/BaseScreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "client/menu/components/MenuButton.h"
#include "client/menu/styles/StyleManager.h"

BaseScreen::BaseScreen(QWidget* parent): QWidget(parent), mainLayout(new QVBoxLayout(this)) {
    mainLayout->setContentsMargins(Theme::SCREEN_MARGIN_H, Theme::SCREEN_MARGIN_V,
                                   Theme::SCREEN_MARGIN_H, Theme::SCREEN_MARGIN_V);
    mainLayout->setSpacing(Theme::SCREEN_SPACING);
}

QLabel* BaseScreen::createTitle(const QString& text) {
    auto* title = new QLabel(text, this);
    // El objectName permite darle un estilo propio desde la hoja global (#titleLabel).
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    return title;
}

void BaseScreen::addBackButton() {
    auto* backButton = new MenuButton("← Volver", this);
    backButton->setBack();
    connect(backButton, &QPushButton::clicked, this, &BaseScreen::backRequested);

    headerRow = new QHBoxLayout();
    headerRow->addWidget(backButton, 0, Qt::AlignVCenter);
    mainLayout->addLayout(headerRow);
}

void BaseScreen::setTitle(const QString& text) {
    QLabel* title = createTitle(text);
    if (!headerRow) {
        mainLayout->addWidget(title);
        return;
    }
    // Un espacio del mismo ancho que "Volver" a la derecha: el titulo queda centrado.
    headerRow->addWidget(title, 1, Qt::AlignVCenter);
    headerRow->addSpacing(Theme::BACK_BUTTON_WIDTH);
}
