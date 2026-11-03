#include "client/menu/components/MenuButton.h"

#include "client/menu/styles/StyleManager.h"

MenuButton::MenuButton(const QString& text, QWidget* parent): QPushButton(text, parent) {
    // El ancho coincide con el sprite de madera: asi no se escala.
    setFixedWidth(Theme::MENU_COLUMN_WIDTH);
    setFixedHeight(Theme::BUTTON_MIN_HEIGHT);
    setCursor(Qt::PointingHandCursor);
}

void MenuButton::setBack() {
    setObjectName("backButton");
    setFixedSize(Theme::BACK_BUTTON_WIDTH, Theme::BACK_BUTTON_HEIGHT);
}

void MenuButton::setSmall() {
    setObjectName("smallButton");
    setFixedWidth(Theme::SMALL_BUTTON_WIDTH);
}
