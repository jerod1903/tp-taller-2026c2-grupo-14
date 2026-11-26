#include "controlador_nombres.h"

#include <map>
#include <string>
#include <utility>

#include "client/constants.h"
#include "client/gui/rutas.h"

ControladorNombres::ControladorNombres(SDL2pp::Renderer& renderer):
        renderer(renderer), fuente(RUTA_FUENTE, TAMANO_FUENTE_NOMBRE) {}

void ControladorNombres::actualizar(const EstadoJuego& estado) {
    for (const auto& [id, info]: estado.info_jugadores) {
        const std::string& nombre = info.characterName;
        if (nombre.empty() || ya_tiene_cartel(id, nombre)) {
            continue;
        }
        if (nombres.contains(id)) {
            nombres.at(id) = crear_cartel(nombre);
        } else {
            nombres.emplace(id, crear_cartel(nombre));
        }
    }
}

bool ControladorNombres::ya_tiene_cartel(uint32_t id, const std::string& nombre) const {
    return nombres.contains(id) && nombres.at(id).texto == nombre;
}

ControladorNombres::CartelNombre ControladorNombres::crear_cartel(const std::string& nombre) {
    SDL2pp::Texture letras = fuente.renderizar(renderer, nombre, COLOR_NOMBRE);
    SDL2pp::Texture sombra = fuente.renderizar(renderer, nombre, COLOR_SOMBRA_NOMBRE);
    return CartelNombre{nombre, std::move(letras), std::move(sombra)};
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
