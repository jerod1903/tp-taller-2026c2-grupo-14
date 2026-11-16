#include "client/gui/chat/fuente.h"

#include <string>

Fuente::Fuente(const char* ruta, int tamano): fuente(ruta, tamano) {}

SDL2pp::Texture Fuente::renderizar(SDL2pp::Renderer& renderer, const std::string& texto,
                                   const SDL_Color& color) {
    return SDL2pp::Texture(renderer, fuente.RenderUTF8_Solid(texto, color));
}

int Fuente::ancho(const std::string& texto) const { return fuente.GetSizeUTF8(texto).x; }

int Fuente::alto_linea() const { return fuente.GetLineSkip(); }
