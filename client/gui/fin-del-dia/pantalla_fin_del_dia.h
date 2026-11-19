#ifndef PANTALLA_FIN_DEL_DIA_H
#define PANTALLA_FIN_DEL_DIA_H

#include <string>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/gui/chat/fuente.h"
#include "common/game_dto.h"

// La pantalla de fin de día, encima del juego oscurecido: el día que terminó, si durmió en su
// cama y el botón Ok. Después de darlo, en lugar del botón, cuántos dieron el Ok.
class PantallaFinDelDia {
public:
    PantallaFinDelDia(SDL2pp::Renderer& renderer, Fuente& fuente, Fuente& fuente_titulo);
    // true si con este evento dio el Ok: click en el botón o Espacio
    bool leer(const SDL_Event& evento, const SDL2pp::Point& tamano_ventana) const;
    // `listo`: si ya dio el Ok
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana,
                 const DayEnded& fin, bool listo);

private:
    SDL2pp::Rect boton(const SDL2pp::Point& tamano_ventana) const;
    void dibujar_texto(SDL2pp::Renderer& renderer, Fuente& fuente, const std::string& texto,
                       const SDL_Color& color, const SDL2pp::Point& centro);
    SDL2pp::Texture textura_boton;
    Fuente& fuente;
    Fuente& fuente_titulo;
};

#endif
