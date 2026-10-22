#include "client/menu/components/MenuButton.h"

#include "client/menu/styles/StyleManager.h"

MenuButton::MenuButton(const QString& text, QWidget* parent): QPushButton(text, parent) {
    setMinimumHeight(Theme::BUTTON_MIN_HEIGHT);
    setCursor(Qt::PointingHandCursor);
}
