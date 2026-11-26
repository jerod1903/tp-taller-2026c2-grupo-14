#ifndef CONTROLADOR_NOMBRES_H
#define CONTROLADOR_NOMBRES_H

#include <cstdint>
#include <map>
#include <string>
#include <utility>

#include <SDL2pp/SDL2pp.hh>

#include "client/game/estado_juego.h"
#include "client/gui/chat/fuente.h"
#include "client/gui/sprites/entidad_grafica.h"
#include "common/game_dto.h"

class ControladorNombres {
public:
    explicit ControladorNombres(SDL2pp::Renderer& renderer);
    void actualizar(const EstadoJuego& estado);
    void dibujar(const std::map<std::pair<EntityType, uint32_t>, EntidadGrafica>& entidades,
                 const std::string& area, const SDL2pp::Rect& camara, int escala);
    bool tiene_nombre(uint32_t player_id) const;
    int alto(int escala) const;

private:
    struct CartelNombre {
        std::string texto;
        SDL2pp::Texture letras;
        SDL2pp::Texture sombra;
    };
    bool ya_tiene_cartel(uint32_t id, const std::string& nombre) const;
    CartelNombre crear_cartel(const std::string& nombre);
    SDL2pp::Renderer& renderer;
    Fuente fuente;
    std::map<uint32_t, CartelNombre> nombres;
};

#endif
