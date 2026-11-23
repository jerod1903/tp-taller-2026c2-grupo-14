#include "client/gui/menu-pausa/menu_pausa.h"

#include <string>

#include "client/constants.h"
#include "client/gui/fondo/oscurecer_ventana.h"
#include "client/gui/rutas.h"

MenuPausa::MenuPausa(SDL2pp::Renderer& renderer, Fuente& fuente_botones, Fuente& fuente_titulo):
        textura_boton(renderer, RUTA_BOTON_MENU),
        textura_deshabilitado(renderer, RUTA_BOTON_DESHABILITADO),
        fuente_botones(fuente_botones),
        fuente_titulo(fuente_titulo) {}

bool MenuPausa::abierto() const { return visible; }

void MenuPausa::abrir() {
    visible = true;
    seleccionada = 0;
}

std::optional<OpcionPausa> MenuPausa::leer(const SDL_Event& evento,
                                           const SDL2pp::Point& tamano_ventana) {
    if (evento.type == SDL_MOUSEMOTION) {
        std::optional<std::size_t> indice =
                boton_en(SDL2pp::Point(evento.motion.x, evento.motion.y), tamano_ventana);
        if (indice) {
            seleccionada = *indice;
        }
        return std::nullopt;
    }
    if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT) {
        std::optional<std::size_t> indice =
                boton_en(SDL2pp::Point(evento.button.x, evento.button.y), tamano_ventana);
        if (!indice) {
            return std::nullopt;
        }
        seleccionada = *indice;
        return elegir();
    }
    if (evento.type != SDL_KEYDOWN || evento.key.repeat) {
        return std::nullopt;
    }
    SDL_Keycode tecla = evento.key.keysym.sym;
    if (tecla == SDLK_ESCAPE) {
        visible = false;
        return OpcionPausa::Reanudar;
    }
    if (tecla == SDLK_UP || tecla == SDLK_w) {
        mover(false);
    } else if (tecla == SDLK_DOWN || tecla == SDLK_s) {
        mover(true);
    } else if (tecla == SDLK_RETURN) {
        return elegir();
    }
    return std::nullopt;
}

void MenuPausa::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana) {
    oscurecer_ventana(renderer, tamano_ventana);
    SDL2pp::Rect primero = boton(0, tamano_ventana);
    dibujar_texto(renderer, fuente_titulo, TITULO_PAUSA, COLOR_TITULO_PAUSA,
                  SDL2pp::Point(tamano_ventana.x / 2, primero.y - SEPARACION_TITULO_PAUSA));
    for (std::size_t i = 0; i < BOTONES_PAUSA.size(); ++i) {
        const BotonPausa& datos = BOTONES_PAUSA[i];
        SDL2pp::Rect rect = boton(i, tamano_ventana);
        SDL2pp::Point centro(rect.x + rect.w / 2, rect.y + rect.h / 2);
        if (!datos.habilitado) {
            renderer.Copy(textura_deshabilitado, SDL2pp::NullOpt, rect);
            dibujar_texto(renderer, fuente_botones, datos.texto, COLOR_TEXTO_DESHABILITADO, centro);
            continue;
        }
        Uint8 brillo = i == seleccionada ? BRILLO_SELECCIONADO : BRILLO_NO_SELECCIONADO;
        textura_boton.SetColorMod(brillo, brillo, brillo);
        renderer.Copy(textura_boton, SDL2pp::NullOpt, rect);
        dibujar_texto(renderer, fuente_botones, datos.texto, COLOR_TEXTO_BOTON, centro);
    }
}

SDL2pp::Rect MenuPausa::boton(std::size_t indice, const SDL2pp::Point& tamano_ventana) const {
    int ancho = textura_boton.GetWidth();
    int alto = textura_boton.GetHeight();
    int cantidad = static_cast<int>(BOTONES_PAUSA.size());
    int alto_total = cantidad * alto + (cantidad - 1) * SEPARACION_BOTONES;
    int x = (tamano_ventana.x - ancho) / 2;
    int y = (tamano_ventana.y - alto_total) / 2 +
            static_cast<int>(indice) * (alto + SEPARACION_BOTONES);
    return SDL2pp::Rect(x, y, ancho, alto);
}

std::optional<std::size_t> MenuPausa::boton_en(const SDL2pp::Point& punto,
                                               const SDL2pp::Point& tamano_ventana) const {
    for (std::size_t i = 0; i < BOTONES_PAUSA.size(); ++i) {
        if (BOTONES_PAUSA[i].habilitado && boton(i, tamano_ventana).Contains(punto)) {
            return i;
        }
    }
    return std::nullopt;
}

void MenuPausa::mover(bool hacia_abajo) {
    std::size_t cantidad = BOTONES_PAUSA.size();
    do {
        seleccionada = hacia_abajo ? (seleccionada + 1) % cantidad :
                                     (seleccionada + cantidad - 1) % cantidad;
    } while (!BOTONES_PAUSA[seleccionada].habilitado);
}

std::optional<OpcionPausa> MenuPausa::elegir() {
    const BotonPausa& datos = BOTONES_PAUSA[seleccionada];
    if (!datos.habilitado) {
        return std::nullopt;
    }
    visible = false;
    return datos.opcion;
}

void MenuPausa::dibujar_texto(SDL2pp::Renderer& renderer, Fuente& fuente, const std::string& texto,
                              const SDL_Color& color, const SDL2pp::Point& centro) {
    SDL2pp::Texture textura = fuente.renderizar(renderer, texto, color);
    SDL2pp::Point tamano = textura.GetSize();
    SDL2pp::Point esquina(centro.x - tamano.x / 2, centro.y - tamano.y / 2);
    renderer.Copy(textura, SDL2pp::NullOpt, SDL2pp::Rect(esquina, tamano));
}
