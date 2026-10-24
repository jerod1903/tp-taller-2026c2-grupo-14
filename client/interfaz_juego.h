#ifndef INTERFAZ_JUEGO_H
#define INTERFAZ_JUEGO_H

#include <cstdint>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "armador_acciones.h"
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
    void enviar_accion(const std::vector<uint8_t>& bytes);
    void dibujar();
    SDL2pp::SDL sdl;
    SDL2pp::SDLImage sdl_image;
    SDL2pp::Window ventana;
    SDL2pp::Renderer renderer;
    Caverna caverna;
    Personaje personaje;
    CalculadorCamara calculador_camara;
    ArmadorAcciones armador_acciones;
    bool corriendo = true;
};

#endif
