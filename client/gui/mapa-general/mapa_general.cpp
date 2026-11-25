#include "client/gui/mapa-general/mapa_general.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

#include "client/constants.h"
#include "client/gui/fondo/oscurecer_ventana.h"
#include "client/gui/mapa-general/dibujo_mapa.h"

MapaGeneral::MapaGeneral(SDL2pp::Renderer& renderer, GrillaMapa grilla):
        grilla(std::move(grilla)), textura(textura_de_grilla(renderer, this->grilla)) {}

bool MapaGeneral::abierto() const { return visible; }

void MapaGeneral::abrir() {
    visible = true;
    falta_encuadrar = true;
    arrastrando = false;
}

void MapaGeneral::leer(const SDL_Event& evento) {
    if (evento.type == SDL_MOUSEWHEEL && evento.wheel.y != 0) {
        int x = 0;
        int y = 0;
        SDL_GetMouseState(&x, &y);
        acercar(evento.wheel.y > 0, SDL2pp::Point(x, y));
    } else if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_MIDDLE) {
        arrastrando = true;
    } else if (evento.type == SDL_MOUSEBUTTONUP && evento.button.button == SDL_BUTTON_MIDDLE) {
        arrastrando = false;
    } else if (evento.type == SDL_MOUSEMOTION && arrastrando) {
        origen_x += static_cast<float>(evento.motion.xrel);
        origen_y += static_cast<float>(evento.motion.yrel);
    } else if (evento.type == SDL_KEYDOWN && !evento.key.repeat &&
               (evento.key.keysym.sym == SDLK_m || evento.key.keysym.sym == SDLK_ESCAPE)) {
        visible = false;
    }
}

void MapaGeneral::mostrar_entero(const SDL2pp::Point& tamano_ventana) {
    const float ancho = static_cast<float>(std::max(1, grilla.ancho));
    const float alto = static_cast<float>(std::max(1, grilla.alto));
    const float lugar = 1 - 2 * MARGEN_MAPA_GENERAL;
    escala_entera = std::min(tamano_ventana.x * lugar / ancho, tamano_ventana.y * lugar / alto);
    escala = escala_entera;
    origen_x = (static_cast<float>(tamano_ventana.x) - ancho * escala) / 2;
    origen_y = (static_cast<float>(tamano_ventana.y) - alto * escala) / 2;
    falta_encuadrar = false;
}

void MapaGeneral::acercar(bool acercando, const SDL2pp::Point& mouse) {
    const float nueva = std::clamp(acercando ? escala * ZOOM_PASO : escala / ZOOM_PASO,
                                   escala_entera, escala_entera * ZOOM_MAXIMO);
    // La casilla bajo el mouse sigue bajo el mouse
    const float casilla_x = (static_cast<float>(mouse.x) - origen_x) / escala;
    const float casilla_y = (static_cast<float>(mouse.y) - origen_y) / escala;
    origen_x = static_cast<float>(mouse.x) - casilla_x * nueva;
    origen_y = static_cast<float>(mouse.y) - casilla_y * nueva;
    escala = nueva;
}

void MapaGeneral::dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Point& tamano_ventana,
                          const EstadoJuego& estado, ConstructorCapas& caras,
                          std::map<uint32_t, CartelNombre>& nombres) {
    if (falta_encuadrar) {
        mostrar_entero(tamano_ventana);
    }
    oscurecer_ventana(renderer, tamano_ventana);
    renderer.Copy(textura, SDL2pp::NullOpt,
                  SDL2pp::Rect(static_cast<int>(std::lround(origen_x)),
                               static_cast<int>(std::lround(origen_y)),
                               static_cast<int>(std::lround(grilla.ancho * escala)),
                               static_cast<int>(std::lround(grilla.alto * escala))));
    recorrer_jugadores(estado.jugadores, estado.id_propio,
                       [&](const PlayerState& jugador, bool propio) {
                           dibujar_jugador(renderer, jugador, propio, estado, caras, nombres);
                       });
}

void MapaGeneral::dibujar_jugador(SDL2pp::Renderer& renderer, const PlayerState& jugador,
                                  bool propio, const EstadoJuego& estado, ConstructorCapas& caras,
                                  std::map<uint32_t, CartelNombre>& nombres) const {
    std::optional<SDL_FPoint> casilla = jugador_en_grilla(grilla, jugador);
    if (!casilla) {
        return;
    }
    const int lado = std::max(propio ? LADO_MINIMO_CARA_PROPIA : LADO_MINIMO_CARA,
                              static_cast<int>((propio ? LADO_CARA_PROPIA : LADO_CARA) * escala));
    const SDL_FPoint centro{origen_x + casilla->x * escala, origen_y + casilla->y * escala};
    dibujar_jugador_en_mapa(renderer, caras, estado.info_jugadores, jugador.playerId, centro, lado);
    auto cartel = nombres.find(jugador.playerId);
    if (cartel != nombres.end()) {
        dibujar_cartel(renderer, cartel->second, static_cast<int>(std::lround(centro.x)),
                       static_cast<int>(std::lround(centro.y)) + lado / 2 + SEPARACION_NOMBRE);
    }
}
