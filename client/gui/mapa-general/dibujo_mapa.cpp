#include "client/gui/mapa-general/dibujo_mapa.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "client/constants.h"

namespace {

// Un círculo relleno, de a una fila por vez (SDL no dibuja círculos)
void dibujar_circulo(SDL2pp::Renderer& renderer, const SDL_FPoint& centro, int radio,
                     const SDL_Color& color) {
    renderer.SetDrawColor(color.r, color.g, color.b, color.a);
    const int cx = static_cast<int>(std::lround(centro.x));
    const int cy = static_cast<int>(std::lround(centro.y));
    for (int dy = -radio; dy <= radio; ++dy) {
        const int dx = static_cast<int>(std::sqrt(static_cast<float>(radio * radio - dy * dy)));
        renderer.DrawLine(cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

}  // namespace

SDL2pp::Texture textura_de_grilla(SDL2pp::Renderer& renderer, const GrillaMapa& grilla) {
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

std::optional<SDL_FPoint> jugador_en_grilla(const GrillaMapa& grilla, const PlayerState& jugador) {
    auto lugar = grilla.areas.find(jugador.areaId);
    if (lugar == grilla.areas.end()) {
        return std::nullopt;
    }
    // El servidor manda el centro del jugador en milésimas de casilla
    return SDL_FPoint{static_cast<float>(lugar->second.x) +
                              static_cast<float>(jugador.x) / static_cast<float>(POSITION_SCALE),
                      static_cast<float>(lugar->second.y) +
                              static_cast<float>(jugador.y) / static_cast<float>(POSITION_SCALE)};
}

void recorrer_jugadores(const std::vector<PlayerState>& jugadores, uint32_t id_propio,
                        const std::function<void(const PlayerState&, bool)>& hacer) {
    const PlayerState* propio = nullptr;
    for (const PlayerState& jugador: jugadores) {
        if (jugador.playerId == id_propio) {
            propio = &jugador;
        } else {
            hacer(jugador, false);
        }
    }
    if (propio) {
        hacer(*propio, true);
    }
}

void dibujar_jugador_en_mapa(SDL2pp::Renderer& renderer, ConstructorCapas& caras,
                             const std::map<uint32_t, PlayerInfo>& info_jugadores,
                             uint32_t player_id, const SDL_FPoint& centro, int lado) {
    auto info = info_jugadores.find(player_id);
    if (info == info_jugadores.end()) {
        const int radio = std::max(1, lado / 4);
        dibujar_circulo(renderer, centro, radio + 1, COLOR_BORDE_PUNTO);
        dibujar_circulo(renderer, centro, radio, COLOR_PUNTO_JUGADOR);
        return;
    }
    SDL2pp::Texture& cara = caras.cara(player_id, info->second.appearance);
    const int esquina_x = static_cast<int>(std::lround(centro.x)) - lado / 2;
    const int esquina_y = static_cast<int>(std::lround(centro.y)) - lado / 2;
    renderer.Copy(cara, SDL2pp::NullOpt, SDL2pp::Rect(esquina_x, esquina_y, lado, lado));
}
