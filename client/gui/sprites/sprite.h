#ifndef SPRITE_H
#define SPRITE_H

#include <optional>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

struct Sprite {
    SDL2pp::Texture* textura;
    SDL2pp::Rect origen;
    SDL2pp::Rect destino;
    bool espejado = false;
    SDL_Color tinte = {255, 255, 255, 255};
    std::optional<int> apoyo = std::nullopt;

    int base() const { return apoyo ? *apoyo : destino.y + destino.h; }

    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara) const {
        if (destino.Intersects(camara)) {
            textura->SetColorMod(tinte.r, tinte.g, tinte.b);
            renderer.Copy(
                    *textura, origen,
                    SDL2pp::Rect(destino.x - camara.x, destino.y - camara.y, destino.w, destino.h),
                    0.0, SDL2pp::NullOpt, espejado ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        }
    }
};

#endif
