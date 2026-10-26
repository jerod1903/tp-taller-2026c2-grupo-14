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
    connect(backButton, &QPushButton::clicked, this, &BaseScreen::backRequested);

    auto* row = new QHBoxLayout();
    row->addWidget(backButton);
    row->addStretch();
    mainLayout->addLayout(row);
}
