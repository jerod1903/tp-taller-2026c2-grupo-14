#ifndef PANEL_CHAT_H
#define PANEL_CHAT_H

#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/game/estado_juego.h"
#include "client/gui/chat/fuente.h"

class PanelChat {
public:
    explicit PanelChat(Fuente& fuente);
    void dibujar(SDL2pp::Renderer& renderer, const std::vector<MensajeChat>& mensajes,
                 const std::optional<std::string>& borrador, const SDL2pp::Point& tamano_ventana,
                 const std::string& nombre_propio, std::chrono::steady_clock::time_point ahora);

private:
    using Linea = std::pair<std::string, SDL_Color>;
    std::vector<Linea> lineas_visibles(const std::vector<MensajeChat>& mensajes, bool abierto,
                                       int ancho_maximo, const std::string& nombre_propio,
                                       std::chrono::steady_clock::time_point ahora) const;
    std::vector<std::string> partir(const std::string& texto, int ancho_maximo) const;
    void dibujar_linea(SDL2pp::Renderer& renderer, const Linea& linea,
                       const SDL2pp::Point& posicion);
    Fuente& fuente;
};

#endif
