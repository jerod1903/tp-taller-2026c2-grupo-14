#ifndef ENTIDAD_GRAFICA_H
#define ENTIDAD_GRAFICA_H

#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/gui/calculadores/calculador_posicion.h"
#include "client/gui/sprites/animador_sprites.h"
#include "client/gui/sprites/capa.h"
#include "client/gui/sprites/pose.h"
#include "client/gui/sprites/sprite.h"
#include "common/game_dto.h"

class EntidadGrafica {
public:
    EntidadGrafica(const FormatoAnimacion& formato, std::vector<Capa> capas);
    // Con una hoja por pose: cambia de una a otra según lo que esté haciendo. Empieza quieta.
    // `poses` tiene que vivir más que la entidad y tener todas las poses.
    explicit EntidadGrafica(const std::map<Pose, Aspecto>& poses);
    void vestir(std::vector<Capa> capas);
    void actualizar(const EntityState& estado, uint32_t tick,
                    std::chrono::duration<float> duracion_tick,
                    std::chrono::steady_clock::time_point ahora);
    // Un jugador inconsciente: se dibuja acostado, girado sobre los pies, y sin globo.
    void tirar(bool tirada);
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
    bool tirada = false;
    CalculadorPosicion posicion;
    AnimadorSprites animador;
    const std::map<Pose, Aspecto>* poses = nullptr;
    Pose pose = Pose::Quieto;

    // Si tiene una hoja por pose y cambió lo que hace, pasa a la de la nueva
    void cambiar_pose(const EntityState& estado);
};

#endif
