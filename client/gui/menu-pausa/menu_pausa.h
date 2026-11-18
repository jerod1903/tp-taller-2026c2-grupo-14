#ifndef MENU_PAUSA_H
#define MENU_PAUSA_H

#include <cstddef>
#include <optional>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/gui/chat/fuente.h"

enum class OpcionPausa { Reanudar, Invitar, Salir };

class MenuPausa {
public:
    MenuPausa(SDL2pp::Renderer& renderer, Fuente& fuente_botones, Fuente& fuente_titulo);
    bool abierto() const;
    void abrir();
    std::optional<OpcionPausa> leer(const SDL_Event& evento, const SDL2pp::Point& tamano_ventana);
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana);

private:
    SDL2pp::Rect boton(std::size_t indice, const SDL2pp::Point& tamano_ventana) const;
    std::optional<std::size_t> boton_en(const SDL2pp::Point& punto,
                                        const SDL2pp::Point& tamano_ventana) const;
    std::optional<OpcionPausa> elegir();
    void dibujar_texto(SDL2pp::Renderer& renderer, Fuente& fuente, const std::string& texto,
                       const SDL_Color& color, const SDL2pp::Point& centro);
    SDL2pp::Texture textura_boton;
    Fuente& fuente_botones;
    Fuente& fuente_titulo;
    bool visible = false;
    std::size_t seleccionada = 0;
};

#endif
