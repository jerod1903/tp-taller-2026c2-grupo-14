#include "client/gui/inventario/panel_cofre.h"

#include <algorithm>

#include "client/constants.h"
#include "client/gui/hud/marco.h"
#include "client/gui/rutas.h"

namespace {
constexpr int PASO = TAMANO_CASILLERO + SEPARACION_CASILLEROS;

// Ancho de una fila de esa cantidad de casilleros
int ancho_fila(int cantidad) { return cantidad > 0 ? cantidad * PASO - SEPARACION_CASILLEROS : 0; }

int filas_cofre(int cantidad) { return (cantidad + COLUMNAS_COFRE - 1) / COLUMNAS_COFRE; }
}  // namespace

PanelCofre::PanelCofre(SDL2pp::Renderer& renderer, BarraInventario& barra):
        barra(barra), marco(renderer, RUTA_MARCO_HUD) {}

void PanelCofre::dibujar(SDL2pp::Renderer& renderer, const std::vector<InventorySlot>& cofre,
                         const InventoryState& inventario, const SDL2pp::Point& tamano_lienzo) {
    cantidad_cofre = static_cast<int>(cofre.size());
    cantidad_jugador = static_cast<int>(inventario.slots.size());
    int ancho = std::max(ancho_fila(std::min(cantidad_cofre, COLUMNAS_COFRE)),
                         ancho_fila(cantidad_jugador)) +
                2 * BORDE_MARCO;
    int alto = filas_cofre(cantidad_cofre) * PASO - SEPARACION_CASILLEROS +
               SEPARACION_GRUPOS_COFRE + TAMANO_CASILLERO + 2 * BORDE_MARCO;
    contorno =
            SDL2pp::Rect((tamano_lienzo.x - ancho) / 2, (tamano_lienzo.y - alto) / 2, ancho, alto);
    dibujar_marco(renderer, marco, contorno, 1);
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    for (int i = 0; i < cantidad_cofre; ++i) {
        barra.dibujar_casillero(renderer, cofre[i], caja(ChestSide::Chest, i), false);
    }
    for (int i = 0; i < cantidad_jugador; ++i) {
        barra.dibujar_casillero(renderer, inventario.slots[i], caja(ChestSide::Player, i),
                                i == inventario.selectedSlot);
    }
}

std::optional<ChestSlotRequest> PanelCofre::casillero_en(const SDL2pp::Point& punto) const {
    for (int i = 0; i < cantidad_cofre; ++i) {
        if (caja(ChestSide::Chest, i).Contains(punto)) {
            return ChestSlotRequest{ChestSide::Chest, static_cast<uint8_t>(i)};
        }
    }
    for (int i = 0; i < cantidad_jugador; ++i) {
        if (caja(ChestSide::Player, i).Contains(punto)) {
            return ChestSlotRequest{ChestSide::Player, static_cast<uint8_t>(i)};
        }
    }
    return std::nullopt;
}

SDL2pp::Rect PanelCofre::caja(ChestSide lado, int indice) const {
    int centro = contorno.x + contorno.w / 2;
    int arriba = contorno.y + BORDE_MARCO;
    if (lado == ChestSide::Chest) {
        int columnas = std::min(cantidad_cofre, COLUMNAS_COFRE);
        int izquierda = centro - ancho_fila(columnas) / 2;
        return SDL2pp::Rect(izquierda + (indice % COLUMNAS_COFRE) * PASO,
                            arriba + (indice / COLUMNAS_COFRE) * PASO, TAMANO_CASILLERO,
                            TAMANO_CASILLERO);
    }
    int izquierda = centro - ancho_fila(cantidad_jugador) / 2;
    int fila_jugador = arriba + filas_cofre(cantidad_cofre) * PASO - SEPARACION_CASILLEROS +
                       SEPARACION_GRUPOS_COFRE;
    return SDL2pp::Rect(izquierda + indice * PASO, fila_jugador, TAMANO_CASILLERO,
                        TAMANO_CASILLERO);
}
