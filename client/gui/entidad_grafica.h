#ifndef ENTIDAD_GRAFICA_H
#define ENTIDAD_GRAFICA_H

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "common/game_dto.h"

#include "animador_sprites.h"
#include "calculador_posicion.h"
#include "capa.h"
#include "sprite.h"

// Cualquier cosa que mueve el servidor y se dibuja animada: un jugador, una vaca, una gallina.
// Solo dibuja: la posicion y la direccion las decide el servidor
class EntidadGrafica {
public:
    // Las hojas de las capas son compartidas entre todas las entidades del mismo tipo
    EntidadGrafica(const FormatoAnimacion& formato, std::vector<Capa> capas);
    // Se llama en cada frame con el estado del ultimo Snapshot
    void actualizar(const EntityState& estado, uint32_t tick,
                    std::chrono::steady_clock::time_point ahora);
    // Una por capa, en orden: cada una se dibuja encima de la anterior
    std::vector<Sprite> sprites() const;
    // En pixeles del mundo, para la camara
    SDL2pp::Point centro() const;
    const std::string& area() const;

private:
    std::vector<Capa> capas;
    int ancho_cuadro;
    int alto_cuadro;
    CalculadorPosicion posicion;
    AnimadorSprites animador;
};

#endif
