#include "client/game/game_loop.h"

#include <chrono>
#include <optional>
#include <thread>
#include <variant>
#include <vector>

#include "client/config/client_config.h"
#include "client/constants.h"
#include "client/menu/backend/ILobbyProtocol.h"

namespace {
// Cuanto se espera el GameEnded despues de LeaveGame antes de dar la conexion por perdida
constexpr std::chrono::seconds ESPERA_FIN_PARTIDA(3);
}  // namespace

GameLoop::GameLoop(Enviador& enviador, Recibidor& recibidor, Interfaz& interfaz,
                   uint32_t id_propio):
        enviador(enviador),
        recibidor(recibidor),
        interfaz(interfaz),
        estado{id_propio, {}, {}, {}} {}

std::optional<EndReason> GameLoop::ejecutar() {
    try {
        while (corriendo) {
            auto inicio = std::chrono::steady_clock::now();
            if (limite_salida && inicio > *limite_salida) {
                return std::nullopt;
            }
            procesar_entrada(interfaz.leer_entrada());
            procesar_mensajes();
            interfaz.dibujar(estado);
            std::this_thread::sleep_until(inicio + client_config().ventana.duracion_frame);
        }
    } catch (const LobbyConnectionError&) {
        return std::nullopt;  // no se pudo mandar: se corto la conexion
    }
    return fin;
}

void GameLoop::procesar_entrada(const Entrada& entrada) {
    if (limite_salida) {
        return;
    }
    if (entrada.salir) {
        // Se avisa al servidor que el jugador sale de la partida
        enviador.salir();
        limite_salida = std::chrono::steady_clock::now() + ESPERA_FIN_PARTIDA;
        return;
    }
    // El servidor sigue moviendo al jugador en la ultima direccion que recibio: solo se avisa
    // cuando cambia, incluido el (0, 0) al soltar las teclas para que frene
    if (entrada.dir_x != ultima_dir_x || entrada.dir_y != ultima_dir_y) {
        enviador.enviar(MoveRequest{entrada.dir_x, entrada.dir_y});
        ultima_dir_x = entrada.dir_x;
        ultima_dir_y = entrada.dir_y;
    }
    if (entrada.cruzar_puerta) {
        enviador.cruzar_puerta();
    }
}

void GameLoop::procesar_mensajes() {
    // Se mira antes de vaciar la cola: si el hilo ya habia terminado, todo lo que mando ya esta
    // en ella (incluido el GameEnded)
    const bool recibidor_vivo = recibidor.is_alive();
    // Se procesa todo lo que llego desde el frame anterior
    std::optional<ServerGameMessage> mensaje;
    while ((mensaje = recibidor.recibir())) {
        if (const auto* ubicacion = std::get_if<LocationData>(&*mensaje)) {
            estado.mundo = ubicacion->worldId;
            estado.max_jugadores = ubicacion->maxPlayers;
            if (ubicacion->tickRate > 0) {
                estado.duracion_tick = std::chrono::duration<float>(1.0f / ubicacion->tickRate);
            }
        } else if (const auto* snapshot = std::get_if<Snapshot>(&*mensaje)) {
            estado.jugadores = snapshot->players;
            estado.entidades = snapshot->entities;
            estado.tick = snapshot->tick;
        } else if (const auto* info = std::get_if<PlayerInfo>(&*mensaje)) {
            estado.info_jugadores.insert_or_assign(info->playerId, *info);
        } else {
            fin = std::get<GameEnded>(*mensaje).reason;  // termino la partida
            corriendo = false;
        }
    }
    // El hilo termina si se corto la conexion: fin queda vacio
    if (!recibidor_vivo) {
        corriendo = false;
    }
}
