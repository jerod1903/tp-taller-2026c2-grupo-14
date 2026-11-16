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
#include "client/gui/sprites/constructor_capas.h"
#include "client/gui/calculadores/calculador_camara.h"
#include "client/gui/sprites/capa.h"
#include "client/gui/chat/fuente.h"
#include "client/gui/chat/panel_chat.h"
#include "client/gui/inventario/barra_inventario.h"
#include "client/gui/sprites/globo.h"
#include "client/gui/sprites/entidad_grafica.h"
#include "client/gui/mapa/mapa.h"

class Interfaz {
public:
    Interfaz();
    Entrada leer_entrada();
    void dibujar(const EstadoJuego& estado);

private:
    void actualizar_mapa(const EstadoJuego& estado);
    void actualizar_entidades(const EstadoJuego& estado,
                              std::chrono::steady_clock::time_point ahora);
    void vestir(EntidadGrafica& jugador, uint32_t player_id, const EstadoJuego& estado,
                bool recien_creado);
    void leer_chat(const SDL_Event& evento, Entrada& entrada);
    void ajustar_lienzo();
    void alternar_pantalla_completa();
    static Capa capa_animal(SDL2pp::Texture& hoja, const FormatoAnimacion& formato);
    void dibujar_area(const EntidadGrafica& propio);
    void dibujar_inventario(const InventoryState& inventario);
    void actualizar_sonidos(const EstadoJuego& estado, std::chrono::steady_clock::time_point ahora);
    SDL2pp::SDL sdl;
    ReproductorSonidos reproductor;
    SDL2pp::SDLImage sdl_image;
    SDL2pp::Window ventana;
    SDL2pp::Renderer renderer;
    ConstructorCapas constructor_capas;
    SDL2pp::Texture hoja_vaca;
    SDL2pp::Texture hoja_gallina;
    struct Aspecto {
        const FormatoAnimacion* formato;
        std::vector<Capa> capas;
    };
    std::map<EntityType, Aspecto> aspectos;
    CalculadorCamara calculador_camara;
    std::optional<SDL2pp::Texture> lienzo;
    int escala = 1;
    bool pantalla_completa = false;
    std::optional<Mapa> mapa;
    std::string mundo_del_mapa;
    std::map<std::pair<EntityType, uint32_t>, EntidadGrafica> entidades;
    BarraInventario barra;
    Fuente fuente;
    PanelChat panel_chat;
    std::optional<std::string> borrador;
    Globo globo;
    std::set<uint32_t> vestidos;
    std::optional<uint32_t> tick_escuchado;
    std::optional<PlayerState> propio_anterior;
    bool caminando = false;
    std::chrono::steady_clock::time_point proximo_paso;
    std::optional<int> leche_anterior;
};

#endif
