#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2pp/SDL2pp.hh>

// Algo para dibujar sobre el mapa (una cabaña, un NPC, un jugador): que parte de que textura va en
// que lugar del mundo, en pixeles del tileset. La Interfaz los ordena por su base para que lo que
// esta mas adelante tape a lo de atras.
struct Sprite {
    SDL2pp::Texture* textura;
    SDL2pp::Rect origen;
    SDL2pp::Rect destino;
    bool espejado = false;  // dado vuelta de izquierda a derecha

    // Donde apoya: el borde de abajo
    int base() const { return destino.y + destino.h; }

    // Solo si entra en la camara
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) const {
        if (destino.Intersects(camara)) {
            renderer.Copy(*textura, origen,
                          SDL2pp::Rect(destino.x - camara.x, destino.y - camara.y, destino.w,
                                       destino.h),
                          0.0, SDL2pp::NullOpt, espejado ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        }
    }
};

#endif
