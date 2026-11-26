#include "client/gui/mapa-general/minimapa.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

#include "client/constants.h"
#include "client/gui/hud/marco.h"
#include "client/gui/mapa-general/dibujo_mapa.h"
#include "client/gui/rutas.h"

Minimapa::Minimapa(SDL2pp::Renderer& renderer, GrillaMapa grilla):
        grilla(std::move(grilla)),
        textura(textura_de_grilla(renderer, this->grilla)),
        marco(renderer, RUTA_MARCO_HUD) {}

void Minimapa::dibujar(SDL2pp::Renderer& renderer, const std::vector<PlayerState>& jugadores,
                       const PlayerState& propio, int escala_hud) {
    auto lugar = grilla.areas.find(propio.areaId);
    if (lugar == grilla.areas.end()) {
        return;
    }
    const LugarArea& area = lugar->second;
    // Píxeles por casilla: el área entra entera en el tamaño máximo sin deformarse
    const float escala = std::min(
            static_cast<float>(ANCHO_MAXIMO_MINIMAPA * escala_hud) / static_cast<float>(area.ancho),
            static_cast<float>(ALTO_MAXIMO_MINIMAPA * escala_hud) / static_cast<float>(area.alto));
    const int borde = BORDE_MARCO * escala_hud;
    const int margen = MARGEN_HUD * escala_hud;
    const SDL2pp::Rect adentro(margen + borde, margen + borde,
                               static_cast<int>(std::lround(area.ancho * escala)),
                               static_cast<int>(std::lround(area.alto * escala)));
    dibujar_marco(renderer, marco,
                  SDL2pp::Rect(margen, margen, adentro.w + 2 * borde, adentro.h + 2 * borde),
                  escala_hud);
    renderer.Copy(textura, SDL2pp::Rect(area.x, area.y, area.ancho, area.alto), adentro);
    recorrer_jugadores(jugadores, propio.playerId, [&](const PlayerState& jugador, bool es_propio) {
        std::optional<SDL_FPoint> casilla = jugador_en_grilla(grilla, jugador);
        if (jugador.areaId != propio.areaId || !casilla) {
            return;
        }
        dibujar_punto_jugador(
                renderer,
                SDL_FPoint{adentro.x + (casilla->x - area.x) * escala,
                           adentro.y + (casilla->y - area.y) * escala},
                (es_propio ? RADIO_PUNTO_PROPIO_MINIMAPA : RADIO_PUNTO_MINIMAPA) * escala_hud);
    });
}
