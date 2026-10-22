#pragma once

#include <map>

class QStackedWidget;
class QWidget;

enum class ScreenId {
    MainMenu,
    NewGame,
    JoinGame,
    MyGames,
    Character,
};

// Registra las pantallas del menu y cambia cual se muestra.
// Las pantallas no lo conocen: solo emiten senales, y MainWindow las conecta con show().
class ScreenManager {
public:
    explicit ScreenManager(QStackedWidget& stack);

    // El stack pasa a ser el parent de la pantalla, asi que Qt se encarga de borrarla.
    void add(ScreenId id, QWidget* screen);
    void show(ScreenId id);

private:
    QStackedWidget& stack;
    std::map<ScreenId, QWidget*> screens;
};
