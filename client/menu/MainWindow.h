#pragma once

#include <QMainWindow>

class QStackedWidget;

// Ventana del menu previo a la partida. Cada pantalla es una pagina del QStackedWidget.
class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QStackedWidget* screens;
};
