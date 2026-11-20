#include "client/gui/fondo/oscurecer_ventana.h"

#include "client/constants.h"

void oscurecer_ventana(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana) {
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(COLOR_FONDO_PAUSA.r, COLOR_FONDO_PAUSA.g, COLOR_FONDO_PAUSA.b,
                          COLOR_FONDO_PAUSA.a);
    renderer.FillRect(SDL2pp::Rect(0, 0, tamano_ventana.x, tamano_ventana.y));
}
