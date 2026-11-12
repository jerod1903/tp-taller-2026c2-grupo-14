#include "entidad_grafica.h"

#include <string>
#include <utility>
#include <vector>

EntidadGrafica::EntidadGrafica(const FormatoAnimacion& formato, std::vector<Capa> capas):
        capas(std::move(capas)),
        ancho_cuadro(formato.ancho_cuadro),
        alto_cuadro(formato.alto_cuadro),
        animador(formato) {}

void EntidadGrafica::vestir(std::vector<Capa> capas) { this->capas = std::move(capas); }

void EntidadGrafica::actualizar(const EntityState& estado, uint32_t tick,
                                std::chrono::steady_clock::time_point ahora) {
    posicion.actualizar(CalculadorPosicion::pies_desde_servidor(estado.x, estado.y), estado.areaId,
                        tick, ahora);
    animador.actualizar(estado.dirX, estado.dirY, ahora);
}

std::vector<Sprite> EntidadGrafica::sprites() const {
    Cuadro cuadro = animador.cuadro();
    const SDL2pp::Point& pies = posicion.pies();
    // El cuadro del cuerpo, centrado en x y apoyado en los pies
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
        // Si el cuerpo esta espejado, lo que lleva encima queda del otro lado del cuadro
        int x = cuadro.espejado ? ancho_cuadro - capa.desplazamiento.x - capa.tamano.x
                                : capa.desplazamiento.x;
        SDL2pp::Rect destino(esquina.x + x, esquina.y + capa.desplazamiento.y + bajada,
                             capa.tamano.x, capa.tamano.y);
        resultado.push_back(Sprite{capa.hoja, origen, destino, espejado, capa.tinte, pies.y});
    }
    return resultado;
}

SDL2pp::Point EntidadGrafica::centro() const {
    const SDL2pp::Point& pies = posicion.pies();
    return SDL2pp::Point(pies.x, pies.y - alto_cuadro / 2);
}

const std::string& EntidadGrafica::area() const { return posicion.area(); }
