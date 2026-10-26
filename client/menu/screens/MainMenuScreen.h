#pragma once

#include "client/menu/screens/BaseScreen.h"

// Pantalla inicial. No sabe a donde lleva cada opcion: solo avisa que se eligio.
class MainMenuScreen: public BaseScreen {
    Q_OBJECT

public:
    explicit MainMenuScreen(QWidget* parent = nullptr);

signals:
    void newGameRequested();
    void joinGameRequested();
    void myGamesRequested();
    void exitRequested();
};
