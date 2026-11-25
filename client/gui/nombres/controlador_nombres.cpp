#include "controlador_nombres.h"

#include <map>
#include <string>
#include <utility>

#include "client/constants.h"
#include "client/gui/rutas.h"

ControladorNombres::ControladorNombres(SDL2pp::Renderer& renderer):
        renderer(renderer), fuente(RUTA_FUENTE, TAMANO_FUENTE_NOMBRE) {}

void ControladorNombres::actualizar(const EstadoJuego& estado) {
    std::erase_if(nombres, [&estado](const auto& par) {
        return !estado.info_jugadores.contains(par.first);
    });
    for (const auto& [id, info]: estado.info_jugadores) {
        if (info.characterName.empty()) {
            continue;
        }
        auto cartel = nombres.find(id);
        if (cartel != nombres.end() && cartel->second.texto == info.characterName) {
            continue;
        }
        nombres.insert_or_assign(
                id, CartelNombre{info.characterName,
                                 fuente.renderizar(renderer, info.characterName, COLOR_NOMBRE),
                                 fuente.renderizar(renderer, info.characterName,
                                                   COLOR_SOMBRA_NOMBRE)});
    }
}

void ControladorNombres::dibujar(
        const std::map<std::pair<EntityType, uint32_t>, EntidadGrafica>& entidades,
        const std::string& area, const SDL2pp::Rect& camara, int escala) {
    for (auto& [id, cartel]: nombres) {
        auto entidad = entidades.find({EntityType::Player, id});
        if (entidad == entidades.end() || entidad->second.area() != area) {
            continue;
        }
        SDL2pp::Point cabeza = entidad->second.cabeza();
        SDL2pp::Point tamano = cartel.letras.GetSize();
        SDL2pp::Point punta((cabeza.x - camara.x) * escala,
                            (cabeza.y - SEPARACION_NOMBRE - camara.y) * escala);
        SDL2pp::Rect destino(punta.x - tamano.x / 2, punta.y - tamano.y, tamano.x, tamano.y);
        renderer.Copy(cartel.sombra, SDL2pp::NullOpt, destino + SDL2pp::Point(1, 1));
        renderer.Copy(cartel.letras, SDL2pp::NullOpt, destino);
    }
}

bool ControladorNombres::tiene_nombre(uint32_t player_id) const {
    return nombres.contains(player_id);
}

int ControladorNombres::alto(int escala) const {
    return (fuente.alto_linea() + escala - 1) / escala + SEPARACION_NOMBRE;
}
