#include "client/gui/inventario/barra_inventario.h"

#include <string>

#include <SDL2/SDL.h>

#include "client/constants.h"
#include "client/gui/rutas.h"

namespace {
void pintar(SDL2pp::Renderer& renderer, const SDL_Color& color) {
    renderer.SetDrawColor(color.r, color.g, color.b, color.a);
}
}

BarraInventario::BarraInventario(SDL2pp::Renderer& renderer) {
    for (const auto& [item, ruta]: RUTAS_ICONOS) {
        iconos.try_emplace(item, renderer, ruta);
    }
}

void BarraInventario::dibujar(SDL2pp::Renderer& renderer, const InventoryState& inventario,
                              const SDL2pp::Point& tamano_lienzo) {
    int cantidad = static_cast<int>(inventario.slots.size());
    int ancho = cantidad * TAMANO_CASILLERO + (cantidad - 1) * SEPARACION_CASILLEROS;
    SDL2pp::Point inicio((tamano_lienzo.x - ancho) / 2,
                         tamano_lienzo.y - TAMANO_CASILLERO - MARGEN_BARRA);
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    for (int i = 0; i < cantidad; ++i) {
        SDL2pp::Rect caja(inicio.x + i * (TAMANO_CASILLERO + SEPARACION_CASILLEROS), inicio.y,
                          TAMANO_CASILLERO, TAMANO_CASILLERO);
        dibujar_casillero(renderer, inventario.slots[i], caja, i == inventario.selectedSlot);
    }
}

void BarraInventario::dibujar_casillero(SDL2pp::Renderer& renderer, const InventorySlot& slot,
                                        const SDL2pp::Rect& caja, bool seleccionado) {
    pintar(renderer, COLOR_FONDO_CASILLERO);
    renderer.FillRect(caja);
    pintar(renderer, seleccionado ? COLOR_SELECCIONADO : COLOR_BORDE_CASILLERO);
    renderer.DrawRect(caja);
    if (seleccionado) {
        renderer.DrawRect(SDL2pp::Rect(caja.x - 1, caja.y - 1, caja.w + 2, caja.h + 2));
    }
    if (slot.item == ItemType::Empty) {
        return;
    }
    dibujar_icono(renderer, slot.item, caja);
    if (slot.quantity > 1) {
        dibujar_numero(renderer, slot.quantity,
                       SDL2pp::Point(caja.x + caja.w - 2, caja.y + caja.h - 2));
    }
}

void BarraInventario::dibujar_icono(SDL2pp::Renderer& renderer, ItemType item,
                                    const SDL2pp::Rect& caja) {
    auto icono = iconos.find(item);
    if (icono == iconos.end()) {
        return;
    }
    SDL2pp::Point tamano = icono->second.GetSize();
    renderer.Copy(icono->second, SDL2pp::NullOpt,
                  SDL2pp::Rect(caja.x + (caja.w - tamano.x) / 2, caja.y + (caja.h - tamano.y) / 2,
                               tamano.x, tamano.y));
}

void BarraInventario::dibujar_numero(SDL2pp::Renderer& renderer, int numero,
                                     const SDL2pp::Point& esquina) {
    std::string digitos = std::to_string(numero);
    int x = esquina.x - static_cast<int>(digitos.size()) * (ANCHO_DIGITO + 1) + 1;
    int y = esquina.y - ALTO_DIGITO + 1;
    for (char digito: digitos) {
        const auto& filas = DIGITOS[digito - '0'];
        for (int fila = 0; fila < ALTO_DIGITO; ++fila) {
            for (int columna = 0; columna < ANCHO_DIGITO; ++columna) {
                if (filas[fila][columna] != '#') {
                    continue;
                }
                pintar(renderer, COLOR_SOMBRA_NUMERO);
                renderer.DrawPoint(x + columna + 1, y + fila + 1);
                pintar(renderer, COLOR_NUMERO);
                renderer.DrawPoint(x + columna, y + fila);
            }
        }
        x += ANCHO_DIGITO + 1;
    }
}
