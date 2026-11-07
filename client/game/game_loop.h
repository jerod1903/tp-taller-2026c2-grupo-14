#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <chrono>
#include <cstdint>
#include <optional>

#include "client/game/entrada.h"
#include "client/game/estado_juego.h"
#include "client/gui/interfaz.h"
#include "client/network/enviador.h"
#include "client/network/recibidor.h"
#include "common/game_dto.h"

// El loop de la partida: en cada frame lee lo que hizo el usuario, se lo avisa al servidor,
// actualiza el estado con lo que llego y le pide a la Interfaz que lo dibuje. No usa SDL.
class GameLoop {
public:
    GameLoop(Enviador& enviador, Recibidor& recibidor, Interfaz& interfaz, uint32_t id_propio);
    // Hasta que llega el GameEnded: devuelve su motivo. nullopt si se corto la conexion o si el
    // servidor no confirmo la salida a tiempo
    std::optional<EndReason> ejecutar();

private:
    // Manda Move solo si cambio la direccion, y LeaveGame si el usuario sale. Despues de salir
    // ya no manda nada: solo se espera el GameEnded, para que la conexion vuelva al lobby
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
    // Desde que se mando LeaveGame, y hasta cuando se espera el GameEnded
    std::optional<std::chrono::steady_clock::time_point> limite_salida;
    std::optional<EndReason> fin;
};

#endif
