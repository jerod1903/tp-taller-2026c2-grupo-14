#include "entidad_grafica.h"

#include <string>
#include <utility>
#include <vector>

EntidadGrafica::EntidadGrafica(const FormatoAnimacion& formato, std::vector<Capa> capas):
        capas(std::move(capas)),
        ancho_cuadro(formato.ancho_cuadro),
        alto_cuadro(formato.alto_cuadro),
        animador(formato) {}

void EntidadGrafica::actualizar(const EntityState& estado, uint32_t tick,
                                std::chrono::steady_clock::time_point ahora) {
    posicion.actualizar(CalculadorPosicion::pies_desde_servidor(estado.x, estado.y), estado.areaId,
                        tick, ahora);
    animador.actualizar(estado.dirX, estado.dirY, ahora);
}

std::vector<Sprite> EntidadGrafica::sprites() const {
    Cuadro cuadro = animador.cuadro();
    const SDL2pp::Point& pies = posicion.pies();
    // Centrado en x y apoyado en los pies
    SDL2pp::Rect destino(pies.x - ancho_cuadro / 2, pies.y - alto_cuadro, ancho_cuadro,
                         alto_cuadro);
    std::vector<Sprite> resultado;
    for (const Capa& capa: capas) {
        SDL2pp::Rect origen((cuadro.columna + capa.columnas_corridas) * ancho_cuadro,
                            cuadro.fila * alto_cuadro, ancho_cuadro, alto_cuadro);
        resultado.push_back(Sprite{capa.hoja, origen, destino, cuadro.espejado});
    }
    return resultado;
}

SDL2pp::Point EntidadGrafica::centro() const {
    const SDL2pp::Point& pies = posicion.pies();
    return SDL2pp::Point(pies.x, pies.y - alto_cuadro / 2);
}

const std::string& EntidadGrafica::area() const { return posicion.area(); }
