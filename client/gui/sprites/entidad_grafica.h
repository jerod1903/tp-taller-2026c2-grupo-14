#ifndef ENTIDAD_GRAFICA_H
#define ENTIDAD_GRAFICA_H

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
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
    explicit EntidadGrafica(const std::map<Pose, Aspecto>& poses);
    void vestir(std::vector<Capa> capas);
    void actualizar(const EntityState& estado, uint32_t tick,
                    std::chrono::duration<float> duracion_tick,
                    std::chrono::steady_clock::time_point ahora);
    void tirar(bool tirada);
    void sostener(std::optional<ItemType> item);
    std::optional<ItemType> en_mano() const;
    std::vector<Sprite> sprites() const;
    SDL2pp::Point centro() const;
    SDL2pp::Point cabeza() const;
    Emote emote() const;
    const std::string& area() const;
    SDL2pp::Point pies() const;
    Orientacion mirando() const;
    std::optional<float> avance_ataque(std::chrono::steady_clock::time_point ahora) const;

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
    EntityAction accion = EntityAction::None;
    std::optional<std::chrono::steady_clock::time_point> inicio_ataque;
    std::optional<ItemType> item_en_mano;
    void cambiar_pose(const EntityState& estado);
};

#endif
