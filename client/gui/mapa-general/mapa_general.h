#ifndef MAPA_GENERAL_H
#define MAPA_GENERAL_H

#include <cstdint>
#include <map>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/game/estado_juego.h"
#include "client/gui/sprites/cartel_nombre.h"
#include "client/gui/sprites/constructor_capas.h"
#include "client/mapa-general/grilla_mapa.h"

// El mapa general que se abre con M, encima del juego oscurecido: todas las áreas del mundo como en
// el editor, con la cara y el nombre de cada jugador. Arranca mostrando el mapa entero; la rueda
// acerca o aleja dejando quieto lo que está bajo el mouse y el botón del medio lo arrastra. Se
// cierra con M o Escape.
class MapaGeneral {
public:
    // La textura se arma una vez: una casilla, un píxel, y se estira al dibujar
    MapaGeneral(SDL2pp::Renderer& renderer, GrillaMapa grilla);

    bool abierto() const;
    // Lo abre mostrando el mapa entero
    void abrir();
    void leer(const SDL_Event& evento);
    // Los jugadores en un interior no se ven: su área no está en el mapa general. caras: de donde
    // salen sus caras; nombres: sus carteles, por playerId
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana,
                 const EstadoJuego& estado, ConstructorCapas& caras,
                 std::map<uint32_t, CartelNombre>& nombres);

private:
    // El mapa entero centrado en la ventana, con su margen
    void mostrar_entero(const SDL2pp::Point& tamano_ventana);
    void acercar(bool acercando, const SDL2pp::Point& mouse);
    void dibujar_jugador(SDL2pp::Renderer& renderer, const PlayerState& jugador, bool propio,
                         const EstadoJuego& estado, ConstructorCapas& caras,
                         std::map<uint32_t, CartelNombre>& nombres) const;

    GrillaMapa grilla;
    SDL2pp::Texture textura;
    bool visible = false;
    // Se calcula al dibujar, que es cuando se sabe el tamaño de la ventana
    bool falta_encuadrar = true;
    // Píxeles por casilla: el del mapa entero y el actual
    float escala_entera = 1;
    float escala = 1;
    // Dónde cae, en la ventana, la esquina de arriba a la izquierda del mapa
    float origen_x = 0;
    float origen_y = 0;
    bool arrastrando = false;
};

#endif
