#include "client/game/game_loop.h"

#include <chrono>
#include <optional>
#include <thread>
#include <variant>

#include "client/constants.h"

GameLoop::GameLoop(Enviador& enviador, Recibidor& recibidor, Interfaz& interfaz,
                   uint32_t id_propio):
        enviador(enviador), recibidor(recibidor), interfaz(interfaz), estado{id_propio, {}, {}} {}

void GameLoop::ejecutar() {
    while (corriendo) {
        auto inicio = std::chrono::steady_clock::now();
        procesar_entrada(interfaz.leer_entrada());
        procesar_mensajes();
        interfaz.dibujar(estado);
        std::this_thread::sleep_until(inicio + DURACION_FRAME);
    }
}

void GameLoop::procesar_entrada(const Entrada& entrada) {
    if (entrada.salir) {
        // Se avisa al servidor que el jugador sale de la partida
        enviador.salir();
        corriendo = false;
        return;
    }
    // El servidor sigue moviendo al jugador en la ultima direccion que recibio: solo se avisa
    // cuando cambia, incluido el (0, 0) al soltar las teclas para que frene
    if (entrada.dir_x != ultima_dir_x || entrada.dir_y != ultima_dir_y) {
        enviador.enviar(MoveRequest{entrada.dir_x, entrada.dir_y});
        ultima_dir_x = entrada.dir_x;
        ultima_dir_y = entrada.dir_y;
    }
}

void GameLoop::procesar_mensajes() {
    // Se procesa todo lo que llego desde el frame anterior
    std::optional<ServerGameMessage> mensaje;
    while ((mensaje = recibidor.recibir())) {
        if (const auto* ubicacion = std::get_if<LocationData>(&*mensaje)) {
            estado.mundo = ubicacion->worldId;
        } else if (const auto* snapshot = std::get_if<Snapshot>(&*mensaje)) {
            estado.jugadores = snapshot->players;
            estado.tick = snapshot->tick;
        } else {
            corriendo = false;  // GameEnded: termino la partida
        }
    }
    // El hilo termina si se corto la conexion
    if (!recibidor.is_alive()) {
        corriendo = false;
    }
}
