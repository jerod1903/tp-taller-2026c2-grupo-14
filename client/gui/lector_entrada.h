#ifndef LECTOR_ENTRADA_H
#define LECTOR_ENTRADA_H

#include <optional>
#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "client/audio/controlador_sonidos.h"
#include "client/game/entrada.h"
#include "client/gui/fin-del-dia/pantalla_fin_del_dia.h"
#include "client/gui/inventario/panel_cofre.h"
#include "client/gui/mapa-general/mapa_general.h"
#include "client/gui/menu-pausa/menu_pausa.h"

struct Vista {
    int escala;
    int escala_interfaz;
    std::optional<SDL2pp::Rect> camara;
    bool fin_del_dia_abierto;
    bool cofre_abierto;
};

class LectorEntrada {
public:
    LectorEntrada(SDL2pp::Window& ventana, SDL2pp::Renderer& renderer,
                  ControladorSonidos& controlador_sonidos, MenuPausa& menu_pausa,
                  PantallaFinDelDia& pantalla_fin_del_dia, PanelCofre& panel_cofre,
                  std::optional<MapaGeneral>& mapa_general);
    Entrada leer(const Vista& vista_actual);
    const std::optional<std::string>& borrador_chat() const;

private:
    void leer_evento(const SDL_Event& evento, Entrada& entrada);
    void leer_tecla(SDL_Keycode tecla, Entrada& entrada);
    void leer_direccion(Entrada& entrada) const;
    bool hay_modo_abierto() const;
    void leer_chat(const SDL_Event& evento, Entrada& entrada);
    void leer_pausa(const SDL_Event& evento, Entrada& entrada);
    void leer_fin_del_dia(const SDL_Event& evento, Entrada& entrada);
    void leer_mapa_general(const SDL_Event& evento);
    void leer_cofre(const SDL_Event& evento, Entrada& entrada);
    bool mapa_general_abierto() const;
    ClickRequest casilla_clickeada(int x, int y) const;
    void alternar_pantalla_completa();
    SDL2pp::Window& ventana;
    SDL2pp::Renderer& renderer;
    ControladorSonidos& controlador_sonidos;
    MenuPausa& menu_pausa;
    PantallaFinDelDia& pantalla_fin_del_dia;
    PanelCofre& panel_cofre;
    std::optional<MapaGeneral>& mapa_general;
    Vista vista{1, 1, std::nullopt, false, false};
    std::optional<std::string> borrador;
    bool pantalla_completa = false;
};

#endif
