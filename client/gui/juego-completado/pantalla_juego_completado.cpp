#include "client/gui/juego-completado/pantalla_juego_completado.h"

#include <string>
#include <vector>

#include "client/constants.h"
#include "client/gui/fondo/oscurecer_ventana.h"
#include "client/gui/rutas.h"

namespace {

// "Ana, dipa y MateCapo"
std::string enumerar(const std::vector<std::string>& nombres) {
    std::string texto;
    for (std::size_t i = 0; i < nombres.size(); ++i) {
        if (i > 0) {
            texto += i + 1 == nombres.size() ? " y " : ", ";
        }
        texto += nombres[i];
    }
    return texto;
}

}  // namespace

PantallaJuegoCompletado::PantallaJuegoCompletado(SDL2pp::Renderer& renderer, Fuente& fuente,
                                                 Fuente& fuente_titulo):
        textura_boton(renderer, RUTA_BOTON_MENU), fuente(fuente), fuente_titulo(fuente_titulo) {}

bool PantallaJuegoCompletado::leer(const SDL_Event& evento,
                                   const SDL2pp::Point& tamano_ventana) const {
    if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT) {
        return boton(tamano_ventana).Contains(SDL2pp::Point(evento.button.x, evento.button.y));
    }
    return evento.type == SDL_KEYDOWN && !evento.key.repeat && evento.key.keysym.sym == SDLK_SPACE;
}

void PantallaJuegoCompletado::dibujar(SDL2pp::Renderer& renderer,
                                      const SDL2pp::Point& tamano_ventana, int dias,
                                      const std::vector<std::string>& jugadores) {
    oscurecer_ventana(renderer, tamano_ventana);
    SDL2pp::Rect rect = boton(tamano_ventana);
    int centro_x = tamano_ventana.x / 2;
    dibujar_texto(renderer, fuente_titulo, TITULO_JUEGO_COMPLETADO, COLOR_TITULO_PAUSA,
                  SDL2pp::Point(centro_x, rect.y - 3 * SEPARACION_TEXTOS_FIN_DEL_DIA));
    std::string en_dias =
            TEXTO_COMPLETADO_EN + std::to_string(dias) + (dias == 1 ? TEXTO_DIA : TEXTO_DIAS);
    dibujar_texto(renderer, fuente, en_dias, COLOR_TEXTO_BOTON,
                  SDL2pp::Point(centro_x, rect.y - 2 * SEPARACION_TEXTOS_FIN_DEL_DIA));
    if (!jugadores.empty()) {
        dibujar_texto(renderer, fuente, TEXTO_JUGARON + enumerar(jugadores), COLOR_TEXTO_BOTON,
                      SDL2pp::Point(centro_x, rect.y - SEPARACION_TEXTOS_FIN_DEL_DIA));
    }
    renderer.Copy(textura_boton, SDL2pp::NullOpt, rect);
    dibujar_texto(renderer, fuente, TEXTO_SEGUIR_JUGANDO, COLOR_TEXTO_BOTON,
                  SDL2pp::Point(centro_x, rect.y + rect.h / 2));
}

SDL2pp::Rect PantallaJuegoCompletado::boton(const SDL2pp::Point& tamano_ventana) const {
    int ancho = textura_boton.GetWidth();
    int alto = textura_boton.GetHeight();
    return SDL2pp::Rect((tamano_ventana.x - ancho) / 2, (tamano_ventana.y - alto) / 2, ancho, alto);
}

void PantallaJuegoCompletado::dibujar_texto(SDL2pp::Renderer& renderer, Fuente& fuente,
                                            const std::string& texto, const SDL_Color& color,
                                            const SDL2pp::Point& centro) {
    SDL2pp::Texture textura = fuente.renderizar(renderer, texto, color);
    SDL2pp::Point tamano = textura.GetSize();
    SDL2pp::Point esquina(centro.x - tamano.x / 2, centro.y - tamano.y / 2);
    renderer.Copy(textura, SDL2pp::NullOpt, SDL2pp::Rect(esquina, tamano));
}
