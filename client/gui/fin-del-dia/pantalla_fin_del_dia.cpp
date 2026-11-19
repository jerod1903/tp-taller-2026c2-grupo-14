#include "client/gui/fin-del-dia/pantalla_fin_del_dia.h"

#include <string>

#include "client/constants.h"
#include "client/gui/rutas.h"

PantallaFinDelDia::PantallaFinDelDia(SDL2pp::Renderer& renderer, Fuente& fuente,
                                     Fuente& fuente_titulo):
        textura_boton(renderer, RUTA_BOTON_MENU), fuente(fuente), fuente_titulo(fuente_titulo) {}

bool PantallaFinDelDia::leer(const SDL_Event& evento, const SDL2pp::Point& tamano_ventana) const {
    if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT) {
        return boton(tamano_ventana).Contains(SDL2pp::Point(evento.button.x, evento.button.y));
    }
    return evento.type == SDL_KEYDOWN && !evento.key.repeat && evento.key.keysym.sym == SDLK_SPACE;
}

void PantallaFinDelDia::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana,
                                const DayEnded& fin, bool listo) {
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(COLOR_FONDO_PAUSA.r, COLOR_FONDO_PAUSA.g, COLOR_FONDO_PAUSA.b,
                          COLOR_FONDO_PAUSA.a);
    renderer.FillRect(SDL2pp::Rect(0, 0, tamano_ventana.x, tamano_ventana.y));
    SDL2pp::Rect rect = boton(tamano_ventana);
    int centro_x = tamano_ventana.x / 2;
    dibujar_texto(renderer, fuente_titulo, TITULO_FIN_DEL_DIA + std::to_string(fin.day),
                  COLOR_TITULO_PAUSA,
                  SDL2pp::Point(centro_x, rect.y - 2 * SEPARACION_TEXTOS_FIN_DEL_DIA));
    dibujar_texto(renderer, fuente, fin.slept ? TEXTO_DURMIO : TEXTO_NO_DURMIO, COLOR_TEXTO_BOTON,
                  SDL2pp::Point(centro_x, rect.y - SEPARACION_TEXTOS_FIN_DEL_DIA));
    SDL2pp::Point centro_boton(centro_x, rect.y + rect.h / 2);
    if (listo) {
        dibujar_texto(renderer, fuente,
                      TEXTO_ESPERANDO + std::string(" (") + std::to_string(fin.ready) + "/" +
                              std::to_string(fin.total) + ")",
                      COLOR_TEXTO_BOTON, centro_boton);
        return;
    }
    renderer.Copy(textura_boton, SDL2pp::NullOpt, rect);
    dibujar_texto(renderer, fuente, TEXTO_OK, COLOR_TEXTO_BOTON, centro_boton);
}

SDL2pp::Rect PantallaFinDelDia::boton(const SDL2pp::Point& tamano_ventana) const {
    int ancho = textura_boton.GetWidth();
    int alto = textura_boton.GetHeight();
    return SDL2pp::Rect((tamano_ventana.x - ancho) / 2, (tamano_ventana.y - alto) / 2, ancho, alto);
}

void PantallaFinDelDia::dibujar_texto(SDL2pp::Renderer& renderer, Fuente& fuente,
                                      const std::string& texto, const SDL_Color& color,
                                      const SDL2pp::Point& centro) {
    SDL2pp::Texture textura = fuente.renderizar(renderer, texto, color);
    SDL2pp::Point tamano = textura.GetSize();
    SDL2pp::Point esquina(centro.x - tamano.x / 2, centro.y - tamano.y / 2);
    renderer.Copy(textura, SDL2pp::NullOpt, SDL2pp::Rect(esquina, tamano));
}
