#include "client/gui/inventario/barra_inventario.h"

#include <optional>
#include <string>

#include <SDL2/SDL.h>

#include "client/constants.h"
#include "client/gui/rutas.h"

namespace {
void pintar(SDL2pp::Renderer& renderer, const SDL_Color& color) {
    renderer.SetDrawColor(color.r, color.g, color.b, color.a);
}

SDL2pp::Rect cuadro_cultivo(CropType cultivo, int columna) {
    return SDL2pp::Rect(columna * TAMANO_CUADRO_CULTIVO,
                        FILAS_CULTIVOS.at(cultivo) * TAMANO_CUADRO_CULTIVO, TAMANO_CUADRO_CULTIVO,
                        TAMANO_CUADRO_CULTIVO);
}

std::optional<SDL2pp::Rect> recorte_cultivo(const InventorySlot& slot) {
    auto semilla = SEMILLAS.find(slot.item);
    if (semilla != SEMILLAS.end()) {
        return cuadro_cultivo(semilla->second, COLUMNA_SEMILLAS);
    }
    auto producto = PRODUCTOS_CULTIVO.find(slot.item);
    if (producto != PRODUCTOS_CULTIVO.end()) {
        return cuadro_cultivo(producto->second, COLUMNAS_CALIDAD.at(slot.quality));
    }
    return std::nullopt;
}

void copiar_centrado(SDL2pp::Renderer& renderer, SDL2pp::Texture& textura,
                     const SDL2pp::Rect& origen, const SDL2pp::Rect& caja) {
    renderer.Copy(textura, origen,
                  SDL2pp::Rect(caja.x + (caja.w - origen.w) / 2, caja.y + (caja.h - origen.h) / 2,
                               origen.w, origen.h));
}
}  // namespace

BarraInventario::BarraInventario(SDL2pp::Renderer& renderer):
        hoja_cultivos(renderer, RUTA_HOJA_CULTIVOS), hoja_minerales(renderer, RUTA_HOJA_MINERALES) {
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
    dibujar_icono(renderer, slot, caja);
    if (slot.quantity > 1) {
        dibujar_numero(renderer, slot.quantity,
                       SDL2pp::Point(caja.x + caja.w - 2, caja.y + caja.h - 2));
    }
}

void BarraInventario::dibujar_icono(SDL2pp::Renderer& renderer, const InventorySlot& slot,
                                    const SDL2pp::Rect& caja) {
    if (std::optional<SDL2pp::Rect> recorte = recorte_cultivo(slot)) {
        copiar_centrado(renderer, hoja_cultivos, *recorte, caja);
        return;
    }
    auto mineral = ICONOS_MINERALES.find(slot.item);
    if (mineral != ICONOS_MINERALES.end()) {
        copiar_centrado(renderer, hoja_minerales, mineral->second, caja);
        return;
    }
    auto icono = iconos.find(slot.item);
    if (icono == iconos.end()) {
        return;
    }
    copiar_centrado(renderer, icono->second,
                    SDL2pp::Rect(SDL2pp::Point(0, 0), icono->second.GetSize()), caja);
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
