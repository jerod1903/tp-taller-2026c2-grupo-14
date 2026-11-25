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
constexpr std::chrono::seconds ESPERA_FIN_PARTIDA(3);
}

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
    if (entrada.cerrar_juego_completado) {
        estado.juego_completado = false;
    }
}

void GameLoop::procesar_mensajes() {
    const bool recibidor_vivo = recibidor.is_alive();
    std::optional<ServerGameMessage> mensaje;
    while ((mensaje = recibidor.recibir())) {
        if (const auto* ubicacion = std::get_if<LocationData>(&*mensaje)) {
            estado.mundo = ubicacion->worldId;
            estado.max_jugadores = ubicacion->maxPlayers;
            estado.energia_maxima = ubicacion->maxEnergy;
            estado.vida_maxima = ubicacion->maxHealth;
            if (ubicacion->tickRate > 0) {
                estado.duracion_tick = std::chrono::duration<float>(1.0f / ubicacion->tickRate);
            }
        } else if (const auto* snapshot = std::get_if<Snapshot>(&*mensaje)) {
            estado.jugadores = snapshot->players;
            estado.entidades = snapshot->entities;
            estado.tick = snapshot->tick;
            estado.dia = snapshot->day;
            estado.minuto_del_dia = snapshot->minuteOfDay;
            if (estado.fin_del_dia && snapshot->day > estado.fin_del_dia->day) {
                estado.fin_del_dia.reset();
                estado.listo_para_manana = false;
            }
        } else if (const auto* info = std::get_if<PlayerInfo>(&*mensaje)) {
            estado.info_jugadores.insert_or_assign(info->playerId, *info);
        } else if (const auto* inv = std::get_if<InventoryState>(&*mensaje)) {
            estado.inventario = *inv;
        } else if (const auto* chat = std::get_if<ChatMessage>(&*mensaje)) {
            estado.chat.push_back(MensajeChat{*chat, std::chrono::steady_clock::now()});
            if (estado.chat.size() > MAX_MENSAJES_GUARDADOS) {
                estado.chat.erase(estado.chat.begin());
            }
        } else if (const auto* granja = std::get_if<FarmState>(&*mensaje)) {
            estado.granja = *granja;
        } else if (const auto* mina = std::get_if<DungeonState>(&*mensaje)) {
            estado.mina[mina->areaId] = *mina;
        } else if (const auto* fin_del_dia = std::get_if<DayEnded>(&*mensaje)) {
            estado.fin_del_dia = *fin_del_dia;
        } else if (const auto* cofre = std::get_if<ChestState>(&*mensaje)) {
            estado.cofre = cofre->open ? std::optional(cofre->slots) : std::nullopt;
        } else if (std::holds_alternative<GameCompleted>(*mensaje)) {
            estado.juego_completado = true;
        } else {
            fin = std::get<GameEnded>(*mensaje).reason;
            corriendo = false;
        }
    }
    if (!recibidor_vivo) {
        corriendo = false;
    }
}
