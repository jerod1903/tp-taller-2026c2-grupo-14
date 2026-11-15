#ifndef FUENTE_H
#define FUENTE_H

#include <string>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

class Fuente {
public:
    Fuente(const char* ruta, int tamano);
    SDL2pp::Texture renderizar(SDL2pp::Renderer& renderer, const std::string& texto,
                               const SDL_Color& color);
    int ancho(const std::string& texto) const;
    int alto_linea() const;

private:
    SDL2pp::SDLTTF ttf;
    SDL2pp::Font fuente;
};

#endif
