#ifndef SPRITE_H
#define SPRITE_H

#include <optional>

#include <SDL2pp/SDL2pp.hh>

// Algo para dibujar sobre el mapa (una cabaña, un NPC, un jugador): que parte de que textura va en
// que lugar del mundo, en pixeles del tileset. La Interfaz los ordena por su base para que lo que
// esta mas adelante tape a lo de atras.
struct Sprite {
    SDL2pp::Texture* textura;
    SDL2pp::Rect origen;
    SDL2pp::Rect destino;
    bool espejado = false;  // dado vuelta de izquierda a derecha
    // Multiplica cada pixel por este color (blanco: sin cambios). Va en cada sprite porque la
    // textura es compartida: el pelo de cada jugador es de un color distinto
    SDL_Color tinte = {255, 255, 255, 255};
    // Donde apoya, si no es el borde de abajo: las capas de un personaje (pelo, sombrero) apoyan
    // donde el cuerpo, para quedar juntas al ordenar
    std::optional<int> apoyo = std::nullopt;

    // Donde apoya: el borde de abajo, salvo que tenga uno propio
    int base() const { return apoyo ? *apoyo : destino.y + destino.h; }

    // Solo si entra en la camara
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) const {
        if (destino.Intersects(camara)) {
            textura->SetColorMod(tinte.r, tinte.g, tinte.b);
            renderer.Copy(*textura, origen,
                          SDL2pp::Rect(destino.x - camara.x, destino.y - camara.y, destino.w,
                                       destino.h),
                          0.0, SDL2pp::NullOpt, espejado ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        }
    }
};

#endif
