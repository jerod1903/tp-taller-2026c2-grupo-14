#include "client/gui/mapa-general/mapa_general.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "client/constants.h"
#include "client/gui/fondo/oscurecer_ventana.h"

namespace {

// Una casilla, un píxel. Con la grilla vacía (un mapa sin áreas), un píxel de fondo.
SDL2pp::Texture textura_de(SDL2pp::Renderer& renderer, const GrillaMapa& grilla) {
    const int ancho = std::max(1, grilla.ancho);
    const int alto = std::max(1, grilla.alto);
    SDL2pp::Texture textura(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, ancho, alto);
    std::vector<uint8_t> pixeles(static_cast<std::size_t>(ancho) * alto * 3, 0);
    for (std::size_t i = 0; i < grilla.colores.size(); ++i) {
        pixeles[i * 3] = grilla.colores[i].r;
        pixeles[i * 3 + 1] = grilla.colores[i].g;
        pixeles[i * 3 + 2] = grilla.colores[i].b;
    }
    textura.Update(SDL2pp::NullOpt, pixeles.data(), ancho * 3);
    return textura;
}

// Un círculo relleno, de a una fila por vez (SDL no dibuja círculos)
void dibujar_circulo(SDL2pp::Renderer& renderer, float centro_x, float centro_y, int radio,
                     const SDL_Color& color) {
    renderer.SetDrawColor(color.r, color.g, color.b, color.a);
    const int cx = static_cast<int>(std::lround(centro_x));
    const int cy = static_cast<int>(std::lround(centro_y));
    for (int dy = -radio; dy <= radio; ++dy) {
        const int dx = static_cast<int>(std::sqrt(static_cast<float>(radio * radio - dy * dy)));
        renderer.DrawLine(cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

}  // namespace

MapaGeneral::MapaGeneral(SDL2pp::Renderer& renderer, GrillaMapa grilla):
        grilla(std::move(grilla)), textura(textura_de(renderer, this->grilla)) {}

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
                          const std::vector<PlayerState>& jugadores, uint32_t id_propio) {
    if (falta_encuadrar) {
        mostrar_entero(tamano_ventana);
    }
    oscurecer_ventana(renderer, tamano_ventana);
    renderer.Copy(textura, SDL2pp::NullOpt,
                  SDL2pp::Rect(static_cast<int>(std::lround(origen_x)),
                               static_cast<int>(std::lround(origen_y)),
                               static_cast<int>(std::lround(grilla.ancho * escala)),
                               static_cast<int>(std::lround(grilla.alto * escala))));
    // El propio al final, para que quede encima de los demás
    const PlayerState* propio = nullptr;
    for (const PlayerState& jugador: jugadores) {
        if (jugador.playerId == id_propio) {
            propio = &jugador;
        } else {
            dibujar_jugador(renderer, jugador, false);
        }
    }
    if (propio) {
        dibujar_jugador(renderer, *propio, true);
    }
}

void MapaGeneral::dibujar_jugador(SDL2pp::Renderer& renderer, const PlayerState& jugador,
                                  bool propio) const {
    auto lugar = grilla.areas.find(jugador.areaId);
    if (lugar == grilla.areas.end()) {
        return;
    }
    // El servidor manda el centro del jugador en milésimas de casilla
    const float casilla_x = static_cast<float>(lugar->second.x) +
                            static_cast<float>(jugador.x) / static_cast<float>(POSITION_SCALE);
    const float casilla_y = static_cast<float>(lugar->second.y) +
                            static_cast<float>(jugador.y) / static_cast<float>(POSITION_SCALE);
    const float centro_x = origen_x + casilla_x * escala;
    const float centro_y = origen_y + casilla_y * escala;
    const int radio =
            std::max(propio ? RADIO_MINIMO_PUNTO_PROPIO : RADIO_MINIMO_PUNTO,
                     static_cast<int>((propio ? RADIO_PUNTO_PROPIO : RADIO_PUNTO) * escala));
    dibujar_circulo(renderer, centro_x, centro_y, radio + 1, COLOR_BORDE_PUNTO);
    dibujar_circulo(renderer, centro_x, centro_y, radio, COLOR_PUNTO_JUGADOR);
}
