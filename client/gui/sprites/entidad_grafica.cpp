#include "client/gui/sprites/entidad_grafica.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace {
// Acostada: la cabeza queda a la derecha de los pies
constexpr double ANGULO_TIRADA = 90.0;
}  // namespace

EntidadGrafica::EntidadGrafica(const FormatoAnimacion& formato, std::vector<Capa> capas):
        capas(std::move(capas)),
        ancho_cuadro(formato.ancho_cuadro),
        alto_cuadro(formato.alto_cuadro),
        margen_arriba(formato.margen_arriba),
        animador(formato) {}

EntidadGrafica::EntidadGrafica(const std::map<Pose, Aspecto>& poses):
        EntidadGrafica(*poses.at(Pose::Quieto).formato, poses.at(Pose::Quieto).capas) {
    this->poses = &poses;
}

void EntidadGrafica::vestir(std::vector<Capa> capas) { this->capas = std::move(capas); }

void EntidadGrafica::actualizar(const EntityState& estado, uint32_t tick,
                                std::chrono::duration<float> duracion_tick,
                                std::chrono::steady_clock::time_point ahora) {
    posicion.actualizar(CalculadorPosicion::pies_desde_servidor(estado.x, estado.y), estado.areaId,
                        tick, duracion_tick, ahora);
    cambiar_pose(estado);
    animador.actualizar(estado.dirX, estado.dirY, ahora);
    emote_actual = estado.emote;
}

std::vector<Sprite> EntidadGrafica::sprites() const {
    Cuadro cuadro = animador.cuadro();
    const SDL2pp::Point& pies = posicion.pies();
    SDL2pp::Point esquina(pies.x - ancho_cuadro / 2, pies.y - alto_cuadro);
    std::vector<Sprite> resultado;
    for (const Capa& capa: capas) {
        SDL2pp::Rect origen(capa.celda, capa.tamano);
        bool espejado = cuadro.espejado;
        int bajada = 0;
        if (capa.tipo == TipoCapa::PorCuadro) {
            origen.x += cuadro.columna * ancho_cuadro;
            origen.y += cuadro.fila * alto_cuadro;
        } else {
            const auto& [vista, espejada] = capa.vistas.at(cuadro.orientacion);
            origen.y += vista * capa.tamano.y;
            espejado = espejada;
            bajada = cuadro.bajada_cabeza;
        }
        int x = cuadro.espejado ? ancho_cuadro - capa.desplazamiento.x - capa.tamano.x :
                                  capa.desplazamiento.x;
        SDL2pp::Rect destino(esquina.x + x, esquina.y + capa.desplazamiento.y + bajada,
                             capa.tamano.x, capa.tamano.y);
        Sprite sprite{capa.hoja, origen, destino, espejado, capa.tinte, pies.y};
        if (tirada) {
            sprite.angulo = ANGULO_TIRADA;
            sprite.pivote = SDL2pp::Point(pies.x - destino.x, pies.y - destino.y);
        }
        resultado.push_back(sprite);
    }
    return resultado;
}

void EntidadGrafica::cambiar_pose(const EntityState& estado) {
    if (poses == nullptr) {
        return;
    }
    Pose nueva = pose_de(estado);
    if (nueva == pose) {
        return;
    }
    pose = nueva;
    const Aspecto& aspecto = poses->at(nueva);
    capas = aspecto.capas;
    animador.cambiar_formato(*aspecto.formato);
}

SDL2pp::Point EntidadGrafica::centro() const {
    const SDL2pp::Point& pies = posicion.pies();
    return SDL2pp::Point(pies.x, pies.y - alto_cuadro / 2);
}

SDL2pp::Point EntidadGrafica::cabeza() const {
    const SDL2pp::Point& pies = posicion.pies();
    return SDL2pp::Point(pies.x, pies.y - alto_cuadro + margen_arriba);
}

void EntidadGrafica::tirar(bool tirada) { this->tirada = tirada; }

Emote EntidadGrafica::emote() const { return tirada ? Emote::None : emote_actual; }

const std::string& EntidadGrafica::area() const { return posicion.area(); }
