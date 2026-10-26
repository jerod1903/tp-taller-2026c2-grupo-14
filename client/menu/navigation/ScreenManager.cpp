#include "client/menu/navigation/ScreenManager.h"

#include <QStackedWidget>

ScreenManager::ScreenManager(QStackedWidget& stack): stack(stack) {}

void ScreenManager::add(ScreenId id, QWidget* screen) {
    stack.addWidget(screen);
    screens[id] = screen;
}

void ScreenManager::show(ScreenId id) { stack.setCurrentWidget(screens.at(id)); }
