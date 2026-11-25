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

#include "client/audio/controlador_sonidos.h"
#include "client/audio/reproductor_sonidos.h"
#include "client/game/entrada.h"
#include "client/game/estado_juego.h"
#include "client/gui/calculadores/calculador_camara.h"
#include "client/gui/chat/fuente.h"
#include "client/gui/chat/panel_chat.h"
#include "client/gui/fin-del-dia/pantalla_fin_del_dia.h"
#include "client/gui/hud/hud.h"
#include "client/gui/inventario/barra_inventario.h"
#include "client/gui/inventario/panel_cofre.h"
#include "client/gui/lector_entrada.h"
#include "client/gui/mapa-general/mapa_general.h"
#include "client/gui/mapa-general/minimapa.h"
#include "client/gui/mapa/mapa.h"
#include "client/gui/mapa/transicion_area.h"
#include "client/gui/menu-pausa/menu_pausa.h"
#include "client/gui/nombres/controlador_nombres.h"
#include "client/gui/sprites/capa.h"
#include "client/gui/sprites/constructor_capas.h"
#include "client/gui/sprites/entidad_grafica.h"
#include "client/gui/sprites/globo.h"
#include "client/gui/sprites/pose.h"
#include "common/game_dto.h"

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
    void ajustar_lienzo(int ancho_area, int alto_area);
    bool mapa_general_abierto() const;
    static Capa capa_animal(SDL2pp::Texture& hoja, const FormatoAnimacion& formato);
    void dibujar_area(const EntidadGrafica& propio, const FarmState& granja,
                      const std::map<std::string, DungeonState>& mina,
                      std::chrono::steady_clock::time_point ahora);
    std::optional<Sprite> sprite_golpe_espada(const EntidadGrafica& entidad,
                                              std::chrono::steady_clock::time_point ahora);
    std::optional<Sprite> sprite_en_mano(const EntidadGrafica& entidad,
                                         std::chrono::steady_clock::time_point ahora);
    void dibujar_tierra(const FarmState& granja, const SDL2pp::Rect& camara);
    std::optional<Sprite> sprite_objeto(const GroundItem& objeto);
    Sprite sprite_veta(const OreDeposit& veta);
    Sprite sprite_cultivo(const FarmTile& celda);
    void dibujar_aviso(const FarmTile& celda, const SDL2pp::Rect& camara);
    void dibujar_inventario(const EstadoJuego& estado);
    void congelar_lienzo(std::chrono::steady_clock::time_point ahora);
    void oscurecer(uint8_t opacidad);
    SDL2pp::SDL sdl;
    ReproductorSonidos reproductor;
    SDL2pp::SDLImage sdl_image;
    SDL2pp::Window ventana;
    SDL2pp::Renderer renderer;
    ConstructorCapas constructor_capas;
    SDL2pp::Texture hoja_vaca;
    SDL2pp::Texture hoja_gallina;
    SDL2pp::Texture hoja_cultivos;
    SDL2pp::Texture hoja_minerales;
    SDL2pp::Texture icono_regadera;
    std::map<EntityType, Aspecto> aspectos;
    // Las que tienen una hoja por pose (HOJAS_POR_POSE): se dibujan con la de lo que estén
    // haciendo. Las texturas van por ruta.
    std::map<std::string, SDL2pp::Texture> hojas_por_pose;
    std::map<EntityType, std::map<Pose, Aspecto>> aspectos_por_pose;
    CalculadorCamara calculador_camara;
    std::optional<SDL2pp::Texture> lienzo;
    std::optional<SDL2pp::Rect> camara;
    int escala = 1;
    int escala_interfaz = 1;
    std::optional<Mapa> mapa;
    // Se arma junto con el mapa, con el mismo mundo
    std::optional<MapaGeneral> mapa_general;
    std::optional<Minimapa> minimapa;
    std::string mundo_del_mapa;
    std::map<std::pair<EntityType, uint32_t>, EntidadGrafica> entidades;
    std::vector<EntityState> camas;
    BarraInventario barra;
    PanelCofre panel_cofre;  // después de barra: dibuja con sus casilleros
    // Si se está mostrando (se decide al dibujar, con el estado)
    bool cofre_abierto = false;
    Fuente fuente;
    Fuente fuente_titulo;
    ControladorNombres controlador_nombres;
    PanelChat panel_chat;
    MenuPausa menu_pausa;
    PantallaFinDelDia pantalla_fin_del_dia;
    // Si se está mostrando (se decide al dibujar, con el estado)
    bool fin_del_dia_abierto = false;
    Hud hud;
    LectorEntrada lector;
    Globo globo;
    TransicionArea transicion;
    std::optional<SDL2pp::Texture> congelado;
    std::string area_anterior;
    std::map<ItemType, SDL2pp::Texture> objetos_piso;
    std::set<uint32_t> vestidos;
    ControladorSonidos controlador_sonidos;
};

#endif
