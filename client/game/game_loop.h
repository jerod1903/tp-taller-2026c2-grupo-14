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

class GameLoop {
public:
    GameLoop(Enviador& enviador, Recibidor& recibidor, Interfaz& interfaz, uint32_t id_propio);
    std::optional<EndReason> ejecutar();

private:
    void procesar_entrada(const Entrada& entrada);
    void procesar_mensajes();
    Enviador& enviador;
    Recibidor& recibidor;
    Interfaz& interfaz;
    EstadoJuego estado;
    int8_t ultima_dir_x = 0;
    int8_t ultima_dir_y = 0;
    bool corriendo = true;
    std::optional<std::chrono::steady_clock::time_point> limite_salida;
    std::optional<EndReason> fin;
};

#endif
