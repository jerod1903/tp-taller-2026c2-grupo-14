#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/network/enviador.h"
#include "client/network/recibidor.h"
#include "common/game_dto.h"

#include "calculador_camara.h"
#include "mapa.h"
#include "personaje.h"

class Interfaz {
public:
    // id_propio: el playerId que le dio el servidor a este jugador al entrar a la partida
    Interfaz(Enviador& enviador, Recibidor& recibidor, uint32_t id_propio);
    void ejecutar();

private:
    void procesar_eventos();
    void actualizar();
    void procesar_mensaje(const ServerGameMessage& mensaje);
    // Crea los personajes nuevos, mueve los que ya estaban y borra los que no vinieron
    void actualizar_personajes(const std::vector<PlayerState>& jugadores);
    void dibujar();
    Enviador& enviador;
    Recibidor& recibidor;
    uint32_t id_propio;
    SDL2pp::SDL sdl;
    SDL2pp::SDLImage sdl_image;
    SDL2pp::Window ventana;
    SDL2pp::Renderer renderer;
    SDL2pp::Texture sprite_personaje;
    CalculadorCamara calculador_camara;
    // Vacio hasta que el servidor diga en que mundo esta el jugador
    std::optional<Mapa> mapa;
    // id del jugador -> su personaje
    std::map<uint32_t, Personaje> personajes;
    // La ultima direccion que se le mando al servidor; al empezar el jugador esta quieto
    SDL2pp::Point ultima_direccion{0, 0};
    bool corriendo = true;
};

#endif
