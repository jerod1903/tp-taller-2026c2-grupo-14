#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <SDL2pp/SDL2pp.hh>
#include "client/audio/reproductor_sonidos.h"
#include "client/game/entrada.h"
#include "client/game/estado_juego.h"
#include "common/game_dto.h"
#include "constructor_capas.h"
#include "calculador_camara.h"
#include "capa.h"
#include "entidad_grafica.h"
#include "mapa.h"

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
    // Crea las entidades nuevas, mueve las que ya estaban y borra las que no vinieron
    void actualizar_entidades(const EstadoJuego& estado,
                              std::chrono::steady_clock::time_point ahora);
    // Lo dibuja con su apariencia apenas llega su PlayerInfo; mientras tanto, sin ropa
    void vestir(EntidadGrafica& jugador, uint32_t player_id, const EstadoJuego& estado,
                bool recien_creado);
    // El mundo se dibuja en pixeles del tileset sobre el lienzo, que despues se estira a la
    // ventana. Se rehace si cambia el tamaño de la ventana
    void ajustar_lienzo();
    void alternar_pantalla_completa();
    static Capa capa_animal(SDL2pp::Texture& hoja, const FormatoAnimacion& formato);
    // Dibuja el area donde esta `propio`, con la camara siguiendolo
    void dibujar_area(const EntidadGrafica& propio);
    // Hace sonar lo que le paso al jugador propio: pasos mientras camina y la moneda si gano oro
    void actualizar_sonidos(const EstadoJuego& estado, std::chrono::steady_clock::time_point ahora);
    SDL2pp::SDL sdl;
    // Despues de sdl: se arma con SDL ya iniciado y se destruye antes de que se cierre
    ReproductorSonidos reproductor;
    SDL2pp::SDLImage sdl_image;
    SDL2pp::Window ventana;
    SDL2pp::Renderer renderer;
    ConstructorCapas constructor_capas;
    SDL2pp::Texture hoja_vaca;
    SDL2pp::Texture hoja_gallina;
    // Como se dibuja cada tipo de entidad
    struct Aspecto {
        const FormatoAnimacion* formato;
        std::vector<Capa> capas;
    };
    std::map<EntityType, Aspecto> aspectos;
    CalculadorCamara calculador_camara;
    std::optional<SDL2pp::Texture> lienzo;
    int escala = 1;  // cuantos pixeles de la ventana mide cada pixel del lienzo
    bool pantalla_completa = false;
    // Vacio hasta que el servidor diga en que mundo esta el jugador
    std::optional<Mapa> mapa;
    std::string mundo_del_mapa;  // el worldId con el que se armo mapa
    // (tipo, id) -> su dibujo: los ids solo son unicos dentro de cada tipo
    std::map<std::pair<EntityType, uint32_t>, EntidadGrafica> entidades;
    // Los jugadores que ya se dibujan con su apariencia
    std::set<uint32_t> vestidos;
    // Para los sonidos: el jugador propio en el ultimo Snapshot, para compararlo con el siguiente
    std::optional<uint32_t> tick_escuchado;
    std::optional<PlayerState> propio_anterior;
    bool caminando = false;  // tiene direccion y se movio (contra una pared no suenan pasos)
    std::chrono::steady_clock::time_point proximo_paso;
};

#endif
