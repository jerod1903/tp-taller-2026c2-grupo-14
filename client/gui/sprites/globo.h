#ifndef GLOBO_H
#define GLOBO_H

#include <SDL2pp/SDL2pp.hh>

#include "common/game_dto.h"

class Globo {
public:
    explicit Globo(SDL2pp::Renderer& renderer);
    void dibujar(SDL2pp::Renderer& renderer, Emote emote, const SDL2pp::Point& cabeza,
                 const SDL2pp::Rect& camara);
    void dibujar_icono(SDL2pp::Renderer& renderer, SDL2pp::Texture& textura,
                       const SDL2pp::Rect& origen, const SDL2pp::Point& cabeza,
                       const SDL2pp::Rect& camara);

private:
    void dibujar_ventanita(SDL2pp::Renderer& renderer, const SDL2pp::Rect& contenido);
    SDL2pp::Texture puntos;
    SDL2pp::Texture leche;
};

#endif
