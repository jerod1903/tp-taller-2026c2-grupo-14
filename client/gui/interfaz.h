#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "client/game/entrada.h"
#include "client/game/estado_juego.h"
#include "common/game_dto.h"

#include "calculador_camara.h"
#include "mapa.h"
#include "personaje.h"

// La parte de SDL del juego: la ventana, el teclado y el dibujo. No sabe de la red ni del loop:
// el GameLoop le pide la entrada y le pasa el estado a dibujar.
class Interfaz {
public:
    Interfaz();
    // Lo que hizo el usuario desde el frame anterior. F11 alterna la pantalla completa
    Entrada leer_entrada();
    void dibujar(const EstadoJuego& estado);

private:
    // Crea el Mapa la primera vez y cada vez que cambia el mundo
    void actualizar_mapa(const EstadoJuego& estado);
    // Crea los personajes nuevos, mueve los que ya estaban y borra los que no vinieron
    void actualizar_personajes(const EstadoJuego& estado,
                               std::chrono::steady_clock::time_point ahora);
    // El mundo se dibuja en pixeles del tileset sobre el lienzo, que despues se estira a la
    // ventana. Se rehace si cambia el tamaño de la ventana
    void ajustar_lienzo();
    void alternar_pantalla_completa();
    // Dibuja el area donde esta `propio`, con la camara siguiendolo
    void dibujar_area(const Personaje& propio);
    SDL2pp::SDL sdl;
    SDL2pp::SDLImage sdl_image;
    SDL2pp::Window ventana;
    SDL2pp::Renderer renderer;
    SDL2pp::Texture sprite_personaje;
    CalculadorCamara calculador_camara;
    std::optional<SDL2pp::Texture> lienzo;
    int escala = 1;  // cuantos pixeles de la ventana mide cada pixel del lienzo
    bool pantalla_completa = false;
    // Vacio hasta que el servidor diga en que mundo esta el jugador
    std::optional<Mapa> mapa;
    std::string mundo_del_mapa;  // el worldId con el que se armo mapa
    // id del jugador -> su personaje
    std::map<uint32_t, Personaje> personajes;
};

#endif
