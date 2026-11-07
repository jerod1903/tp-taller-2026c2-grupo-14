#pragma once

class IGameView;
class IMenu;

// Alterna entre el menu y la partida. No sabe que el menu es Qt ni que el juego es SDL.
class ClientApp {
public:
    ClientApp(IMenu& menu, IGameView& game);
    // Vuelve cuando el jugador cierra el menu o cuando se corta la conexion.
    void run();

private:
    IMenu& menu;
    IGameView& game;
};
