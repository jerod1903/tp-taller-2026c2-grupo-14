#ifndef ENTIDAD_GRAFICA_H
#define ENTIDAD_GRAFICA_H

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/gui/calculadores/calculador_posicion.h"
#include "client/gui/sprites/animador_sprites.h"
#include "client/gui/sprites/capa.h"
#include "client/gui/sprites/sprite.h"
#include "common/game_dto.h"

class EntidadGrafica {
public:
    EntidadGrafica(const FormatoAnimacion& formato, std::vector<Capa> capas);
    void vestir(std::vector<Capa> capas);
    void actualizar(const EntityState& estado, uint32_t tick,
                    std::chrono::duration<float> duracion_tick,
                    std::chrono::steady_clock::time_point ahora);
    std::vector<Sprite> sprites() const;
    SDL2pp::Point centro() const;
    SDL2pp::Point cabeza() const;
    Emote emote() const;
    const std::string& area() const;

private:
    std::vector<Capa> capas;
    int ancho_cuadro;
    int alto_cuadro;
    int margen_arriba;
    Emote emote_actual = Emote::None;
    CalculadorPosicion posicion;
    AnimadorSprites animador;
};

#endif
