#include "client/game/game_loop.h"

#include <chrono>
#include <cstddef>
#include <map>
#include <optional>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "client/config/client_config.h"
#include "client/constants.h"
#include "client/menu/backend/ILobbyProtocol.h"

namespace {
constexpr std::chrono::seconds ESPERA_FIN_PARTIDA(3);

template <typename T>
std::size_t indice() {
    return ServerGameMessage(std::in_place_type<T>).index();
}
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
            if (limite_salida && inicio > *limite_salida) { // por si no llega ga GameEnded
                return std::nullopt;
            }
            procesar_entrada(interfaz.leer_entrada());
            procesar_mensajes();
            interfaz.dibujar(estado);
            std::this_thread::sleep_until(inicio + client_config().ventana.duracion_frame);
        }
    } catch (const LobbyConnectionError&) {
        return std::nullopt;
    }
    return fin;
}

void GameLoop::procesar_entrada(const Entrada& entrada) {
    if (limite_salida) {
        return;
    }
    if (entrada.salir) {
        enviador.salir();
        limite_salida = std::chrono::steady_clock::now() + ESPERA_FIN_PARTIDA;
        return;
    }
    if (entrada.dir_x != ultima_dir_x || entrada.dir_y != ultima_dir_y) {
        enviador.enviar(MoveRequest{entrada.dir_x, entrada.dir_y});
        ultima_dir_x = entrada.dir_x;
        ultima_dir_y = entrada.dir_y;
    }
    if (entrada.cruzar_puerta) {
        enviador.cruzar_puerta();
    }
    if (entrada.slot_seleccionado) {
        enviador.seleccionar_slot(*entrada.slot_seleccionado);
    }
    if (entrada.mensaje_chat) {
        enviador.enviar_chat(*entrada.mensaje_chat);
    }
    if (entrada.interactuar) {
        enviador.interactuar();
    }
    if (entrada.escribiendo) {
        enviador.escribiendo(*entrada.escribiendo);
    }
    if (entrada.click) {
        enviador.clickear(*entrada.click);
    }
    if (entrada.invitar) {
        enviador.invitar();
    }
    if (entrada.listo_para_manana && estado.fin_del_dia && !estado.listo_para_manana) {
        enviador.listo_para_manana();
        estado.listo_para_manana = true;
    }
    if (entrada.transferencia) {
        enviador.transferir(*entrada.transferencia);
    }
    if (entrada.particion) {
        enviador.partir(*entrada.particion);
    }
    if (entrada.cerrar_cofre && estado.cofre) {
        enviador.cerrar_cofre();
        estado.cofre.reset();
    }
}

void GameLoop::procesar_mensajes() {
    static const std::map<std::size_t, void (GameLoop::*)(const ServerGameMessage&)> acciones = {
            {indice<LocationData>(), &GameLoop::aplicar_ubicacion},
            {indice<Snapshot>(), &GameLoop::aplicar_snapshot},
            {indice<PlayerInfo>(), &GameLoop::aplicar_info_jugador},
            {indice<InventoryState>(), &GameLoop::aplicar_inventario},
            {indice<ChatMessage>(), &GameLoop::aplicar_chat},
            {indice<FarmState>(), &GameLoop::aplicar_granja},
            {indice<DungeonState>(), &GameLoop::aplicar_mina},
            {indice<DayEnded>(), &GameLoop::aplicar_fin_del_dia},
            {indice<ChestState>(), &GameLoop::aplicar_cofre},
            {indice<GameEnded>(), &GameLoop::aplicar_fin_partida},
    };
    const bool recibidor_vivo = recibidor.is_alive();
    std::optional<ServerGameMessage> mensaje;
    while ((mensaje = recibidor.recibir())) {
        (this->*acciones.at(mensaje->index()))(*mensaje);
    }
    if (!recibidor_vivo) {
        corriendo = false;
    }
}

void GameLoop::aplicar_ubicacion(const ServerGameMessage& mensaje) {
    const LocationData& ubicacion = std::get<LocationData>(mensaje);
    estado.mundo = ubicacion.worldId;
    estado.max_jugadores = ubicacion.maxPlayers;
    estado.energia_maxima = ubicacion.maxEnergy;
    estado.vida_maxima = ubicacion.maxHealth;
    if (ubicacion.tickRate > 0) {
        estado.duracion_tick = std::chrono::duration<float>(1.0f / ubicacion.tickRate);
    }
}

void GameLoop::aplicar_snapshot(const ServerGameMessage& mensaje) {
    const Snapshot& snapshot = std::get<Snapshot>(mensaje);
    estado.jugadores = snapshot.players;
    estado.entidades = snapshot.entities;
    estado.tick = snapshot.tick;
    estado.dia = snapshot.day;
    estado.minuto_del_dia = snapshot.minuteOfDay;
    if (estado.fin_del_dia && snapshot.day > estado.fin_del_dia->day) {
        estado.fin_del_dia.reset();
        estado.listo_para_manana = false;
    }
}

void GameLoop::aplicar_info_jugador(const ServerGameMessage& mensaje) {
    const PlayerInfo& info = std::get<PlayerInfo>(mensaje);
    estado.info_jugadores.insert_or_assign(info.playerId, info);
}

void GameLoop::aplicar_inventario(const ServerGameMessage& mensaje) {
    estado.inventario = std::get<InventoryState>(mensaje);
}

void GameLoop::aplicar_chat(const ServerGameMessage& mensaje) {
    estado.chat.push_back(
            MensajeChat{std::get<ChatMessage>(mensaje), std::chrono::steady_clock::now()});
    if (estado.chat.size() > MAX_MENSAJES_GUARDADOS) {
        estado.chat.erase(estado.chat.begin());
    }
}

void GameLoop::aplicar_granja(const ServerGameMessage& mensaje) {
    estado.granja = std::get<FarmState>(mensaje);
}

void GameLoop::aplicar_mina(const ServerGameMessage& mensaje) {
    const DungeonState& mina = std::get<DungeonState>(mensaje);
    estado.mina[mina.areaId] = mina;
}

void GameLoop::aplicar_fin_del_dia(const ServerGameMessage& mensaje) {
    estado.fin_del_dia = std::get<DayEnded>(mensaje);
}

void GameLoop::aplicar_cofre(const ServerGameMessage& mensaje) {
    const ChestState& cofre = std::get<ChestState>(mensaje);
    estado.cofre = cofre.open ? std::optional(cofre.slots) : std::nullopt;
}

void GameLoop::aplicar_fin_partida(const ServerGameMessage& mensaje) {
    fin = std::get<GameEnded>(mensaje).reason;
    corriendo = false;
}
