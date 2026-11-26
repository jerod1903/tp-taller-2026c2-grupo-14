#include "controlador_sonidos.h"

#include <string>

#include "client/audio/musica.h"
#include "client/audio/sonidos_granja.h"
#include "client/config/client_config.h"
#include "client/constants.h"

void ControladorSonidos::actualizar(const EstadoJuego& estado, const PlayerState& propio,
                                    std::chrono::steady_clock::time_point ahora) {
    sonar_recibidos(estado.inventario);
    sonar_granja(estado.granja, propio.areaId);
    reproductor.poner_musica(musica_de(propio.areaId, estado.mina.contains(propio.areaId)));
    sonar_movimiento(propio, estado.tick, ahora);
}

void ControladorSonidos::alternar_musica() {
    reproductor.alternar_musica();
}

bool ControladorSonidos::musica_apagada() const {
    return reproductor.musica_apagada();
}

void ControladorSonidos::sonar_recibidos(const InventoryState& inventario) {
    for (const auto& [item, sonido]: SONIDOS_AL_RECIBIR) {
        int cantidad = 0;
        for (const InventorySlot& slot: inventario.slots) {
            if (slot.item == item) {
                cantidad += slot.quantity;
            }
        }
        auto anterior = cantidades_anteriores.find(item);
        if (anterior != cantidades_anteriores.end() && cantidad > anterior->second) {
            reproductor.reproducir(sonido);
        }
        cantidades_anteriores[item] = cantidad;
    }
}

void ControladorSonidos::sonar_granja(const FarmState& granja, const std::string& area_propia) {
    if (granja_anterior == granja) {
        return;
    }
    if (granja_anterior && granja.areaId == area_propia) {
        for (Sonido sonido: sonidos_granja(*granja_anterior, granja)) {
            reproductor.reproducir(sonido);
        }
    }
    granja_anterior = granja;
}

void ControladorSonidos::sonar_movimiento(const PlayerState& propio, uint32_t tick,
                                          std::chrono::steady_clock::time_point ahora) {
    if (tick != tick_escuchado) {
        if (propio_anterior) {
            bool tiene_direccion = propio.dirX != 0 || propio.dirY != 0;
            bool se_movio = propio.x != propio_anterior->x || propio.y != propio_anterior->y;
            caminando = tiene_direccion && se_movio;
            if (propio.gold > propio_anterior->gold) {
                reproductor.reproducir(Sonido::Moneda);
            }
        }
        propio_anterior = propio;
        tick_escuchado = tick;
    }
    if (caminando && ahora >= proximo_paso) {
        reproductor.reproducir(Sonido::Paso);
        proximo_paso = ahora + client_config().audio.intervalo_paso;
    }
}
