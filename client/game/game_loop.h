#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <cstdint>

#include "client/game/entrada.h"
#include "client/game/estado_juego.h"
#include "client/gui/interfaz.h"
#include "client/network/enviador.h"
#include "client/network/recibidor.h"

// El loop de la partida: en cada frame lee lo que hizo el usuario, se lo avisa al servidor,
// actualiza el estado con lo que llego y le pide a la Interfaz que lo dibuje. No usa SDL.
class GameLoop {
public:
    GameLoop(Enviador& enviador, Recibidor& recibidor, Interfaz& interfaz, uint32_t id_propio);
    void ejecutar();

private:
    // Manda Move solo si cambio la direccion, y LeaveGame si el usuario sale
    void procesar_entrada(const Entrada& entrada);
    // Vacia lo que dejo el Recibidor y actualiza el estado
    void procesar_mensajes();
    Enviador& enviador;
    Recibidor& recibidor;
    Interfaz& interfaz;
    EstadoJuego estado;
    // La ultima direccion que se le mando al servidor; al empezar el jugador esta quieto
    int8_t ultima_dir_x = 0;
    int8_t ultima_dir_y = 0;
    bool corriendo = true;
};

#endif
