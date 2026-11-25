#ifndef PANTALLA_JUEGO_COMPLETADO_H
#define PANTALLA_JUEGO_COMPLETADO_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/gui/chat/fuente.h"

// La felicitación por completar el juego (el centro comunitario), encima del juego oscurecido: en
// cuántos días se completó, quiénes juegan y el botón Seguir jugando. El juego sigue de fondo.
class PantallaJuegoCompletado {
public:
    PantallaJuegoCompletado(SDL2pp::Renderer& renderer, Fuente& fuente, Fuente& fuente_titulo);
    // true si con este evento la cerró: click en el botón o Espacio
    bool leer(const SDL_Event& evento, const SDL2pp::Point& tamano_ventana) const;
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana, int dias,
                 const std::vector<std::string>& jugadores);

private:
    SDL2pp::Rect boton(const SDL2pp::Point& tamano_ventana) const;
    void dibujar_texto(SDL2pp::Renderer& renderer, Fuente& fuente, const std::string& texto,
                       const SDL_Color& color, const SDL2pp::Point& centro);
    SDL2pp::Texture textura_boton;
    Fuente& fuente;
    Fuente& fuente_titulo;
};

#endif
