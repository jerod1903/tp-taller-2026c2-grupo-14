#include "client/menu/MainWindow.h"

#include <QStackedWidget>

namespace {
constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 720;
}  // namespace

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), screens(new QStackedWidget(this)) {
    setWindowTitle("Stardew Valley");
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setCentralWidget(screens);
}
