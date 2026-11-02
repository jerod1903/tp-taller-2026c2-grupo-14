#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <cstdint>
#include <map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/network/enviador.h"
#include "client/network/recibidor.h"
#include "common/estado_personaje.h"

#include "calculador_camara.h"
#include "personaje.h"

class Interfaz {
public:
    Interfaz(Enviador& enviador, Recibidor& recibidor);
    void ejecutar();

private:
    void procesar_eventos();
    void actualizar();
    // Crea los personajes nuevos, mueve los que ya estaban y borra los que no vinieron
    void actualizar_personajes(const std::vector<EstadoPersonaje>& estado);
    void dibujar();
    Enviador& enviador;
    Recibidor& recibidor;
    SDL2pp::SDL sdl;
    SDL2pp::SDLImage sdl_image;
    SDL2pp::Window ventana;
    SDL2pp::Renderer renderer;
    SDL2pp::Texture sprite_personaje;
    CalculadorCamara calculador_camara;
    // id del jugador -> su personaje
    std::map<uint16_t, Personaje> personajes;
    bool corriendo = true;
};

#endif
