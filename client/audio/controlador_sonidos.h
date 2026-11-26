#ifndef CONTROLADOR_SONIDOS_H
#define CONTROLADOR_SONIDOS_H

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "client/audio/reproductor_sonidos.h"
#include "client/game/estado_juego.h"
#include "common/farm_dto.h"
#include "common/game_dto.h"

class ControladorSonidos {
public:
    void actualizar(const EstadoJuego& estado, const PlayerState& propio,
                    std::chrono::steady_clock::time_point ahora);
    void alternar_musica();
    bool musica_apagada() const;

private:
    void sonar_recibidos(const InventoryState& inventario);
    void sonar_granja(const FarmState& granja, const std::string& area_propia);
    void sonar_movimiento(const PlayerState& propio, uint32_t tick,
                          std::chrono::steady_clock::time_point ahora);
    ReproductorSonidos reproductor;
    std::optional<uint32_t> tick_escuchado;
    std::optional<PlayerState> propio_anterior;
    bool caminando = false;
    std::chrono::steady_clock::time_point proximo_paso;
    std::map<ItemType, int> cantidades_anteriores;
    std::optional<FarmState> granja_anterior;
};

#endif
