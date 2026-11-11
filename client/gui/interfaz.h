#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include <SDL2pp/SDL2pp.hh>
#include "client/audio/reproductor_sonidos.h"
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
    // Hace sonar lo que le paso al jugador propio: pasos mientras camina y la moneda si gano oro
    void actualizar_sonidos(const EstadoJuego& estado, std::chrono::steady_clock::time_point ahora);
    SDL2pp::SDL sdl;
    // Despues de sdl: se arma con SDL ya iniciado y se destruye antes de que se cierre
    ReproductorSonidos reproductor;
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
    // Para los sonidos: el jugador propio en el ultimo Snapshot, para compararlo con el siguiente
    std::optional<uint32_t> tick_escuchado;
    std::optional<PlayerState> propio_anterior;
    bool caminando = false;  // tiene direccion y se movio (contra una pared no suenan pasos)
    std::chrono::steady_clock::time_point proximo_paso;
    size_t paso_actual = 0;  // cual de SONIDOS_PASO suena la proxima vez
};

#endif
