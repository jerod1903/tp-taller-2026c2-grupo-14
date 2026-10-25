#ifndef INTERFAZ_JUEGO_H
#define INTERFAZ_JUEGO_H

#include <SDL2pp/SDL2pp.hh>

#include "common/requests.h"
#include "calculador_camara.h"
#include "caverna.h"
#include "personaje.h"

class InterfazJuego {
public:
    InterfazJuego();
    void ejecutar();

private:
    void procesar_eventos();
    void actualizar();
    void enviar_request(const Request& request);
    void dibujar();
    SDL2pp::SDL sdl;
    SDL2pp::SDLImage sdl_image;
    SDL2pp::Window ventana;
    SDL2pp::Renderer renderer;
    Caverna caverna;
    Personaje personaje;
    CalculadorCamara calculador_camara;
    bool corriendo = true;
};

#endif
