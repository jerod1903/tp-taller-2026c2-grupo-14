#include "client/gui/sprites/globo.h"

#include "client/constants.h"
#include "client/gui/rutas.h"

namespace {
void pintar(SDL2pp::Renderer& renderer, const SDL_Color& color) {
    renderer.SetDrawColor(color.r, color.g, color.b, color.a);
}
}  // namespace

Globo::Globo(SDL2pp::Renderer& renderer):
        puntos(renderer, RUTA_GLOBO_ESCRIBIENDO), leche(renderer, RUTA_ICONO_LECHE) {}

void Globo::dibujar(SDL2pp::Renderer& renderer, Emote emote, const SDL2pp::Point& cabeza,
                    const SDL2pp::Rect& camara) {
    SDL2pp::Point punta(cabeza.x - camara.x, cabeza.y - camara.y - SEPARACION_GLOBO);
    if (emote == Emote::Typing) {
        SDL2pp::Point tamano = puntos.GetSize();
        renderer.Copy(puntos, SDL2pp::NullOpt,
                      SDL2pp::Rect(punta.x - tamano.x / 2, punta.y - tamano.y, tamano.x, tamano.y));
    } else if (emote == Emote::CowMilkReady) {
        SDL2pp::Point tamano = leche.GetSize();
        SDL2pp::Rect contenido(punta.x - tamano.x / 2,
                               punta.y - ALTO_PALITO - RELLENO_GLOBO - tamano.y, tamano.x,
                               tamano.y);
        dibujar_ventanita(renderer, contenido);
        renderer.Copy(leche, SDL2pp::NullOpt, contenido);
    }
}

void Globo::dibujar_ventanita(SDL2pp::Renderer& renderer, const SDL2pp::Rect& contenido) {
    SDL2pp::Rect caja(contenido.x - RELLENO_GLOBO, contenido.y - RELLENO_GLOBO,
                      contenido.w + 2 * RELLENO_GLOBO, contenido.h + 2 * RELLENO_GLOBO);
    int medio = caja.x + caja.w / 2;
    int abajo = caja.y + caja.h;
    pintar(renderer, COLOR_BORDE_GLOBO);
    renderer.FillRect(SDL2pp::Rect(caja.x - 1, caja.y - 1, caja.w + 2, caja.h + 2));
    renderer.FillRect(SDL2pp::Rect(medio - 2, abajo, 5, 1));
    renderer.FillRect(SDL2pp::Rect(medio - 1, abajo + 1, 3, 1));
    renderer.FillRect(SDL2pp::Rect(medio, abajo + 2, 1, 1));
    pintar(renderer, COLOR_FONDO_GLOBO);
    renderer.FillRect(caja);
    renderer.FillRect(SDL2pp::Rect(medio - 1, abajo, 3, 1));
    renderer.FillRect(SDL2pp::Rect(medio, abajo + 1, 1, 1));
}
