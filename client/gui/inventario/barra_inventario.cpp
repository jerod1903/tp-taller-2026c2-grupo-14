#include "client/gui/inventario/barra_inventario.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <SDL2/SDL.h>

#include "client/constants.h"
#include "client/gui/rutas.h"

namespace {
void pintar(SDL2pp::Renderer& renderer, const SDL_Color& color) {
    renderer.SetDrawColor(color.r, color.g, color.b, color.a);
}
}

BarraInventario::BarraInventario(SDL2pp::Renderer& renderer) {
    for (const auto& [objeto, ruta]: RUTAS_ICONOS) {
        SDL2pp::Texture textura(renderer, ruta);
        auto recorte = RECORTES_ICONOS.find(objeto);
        SDL2pp::Rect parte = recorte != RECORTES_ICONOS.end()
                                     ? recorte->second
                                     : SDL2pp::Rect(SDL2pp::Point(0, 0), textura.GetSize());
        iconos.emplace(objeto, Icono{std::move(textura), parte});
    }
}

void BarraInventario::dibujar(SDL2pp::Renderer& renderer,
                              const std::vector<std::optional<Casillero>>& casilleros,
                              int seleccionado, const SDL2pp::Point& tamano_lienzo) {
    int cantidad = static_cast<int>(casilleros.size());
    int ancho = cantidad * TAMANO_CASILLERO + (cantidad - 1) * SEPARACION_CASILLEROS;
    SDL2pp::Point inicio((tamano_lienzo.x - ancho) / 2,
                         tamano_lienzo.y - TAMANO_CASILLERO - MARGEN_BARRA);
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    for (int i = 0; i < cantidad; ++i) {
        SDL2pp::Rect caja(inicio.x + i * (TAMANO_CASILLERO + SEPARACION_CASILLEROS), inicio.y,
                          TAMANO_CASILLERO, TAMANO_CASILLERO);
        dibujar_casillero(renderer, casilleros[i], caja, i == seleccionado);
    }
}

void BarraInventario::dibujar_casillero(SDL2pp::Renderer& renderer,
                                        const std::optional<Casillero>& casillero,
                                        const SDL2pp::Rect& caja, bool seleccionado) {
    pintar(renderer, COLOR_FONDO_CASILLERO);
    renderer.FillRect(caja);
    pintar(renderer, seleccionado ? COLOR_SELECCIONADO : COLOR_BORDE_CASILLERO);
    renderer.DrawRect(caja);
    if (seleccionado) {
        renderer.DrawRect(SDL2pp::Rect(caja.x - 1, caja.y - 1, caja.w + 2, caja.h + 2));
    }
    if (!casillero) {
        return;
    }
    dibujar_icono(renderer, casillero->objeto, caja);
    if (casillero->cantidad > 1) {
        dibujar_numero(renderer, casillero->cantidad,
                       SDL2pp::Point(caja.x + caja.w - 2, caja.y + caja.h - 2));
    }
}

void BarraInventario::dibujar_icono(SDL2pp::Renderer& renderer, Objeto objeto,
                                    const SDL2pp::Rect& caja) {
    auto icono = iconos.find(objeto);
    if (icono == iconos.end()) {
        auto color = COLORES_SIN_ICONO.find(objeto);
        if (color != COLORES_SIN_ICONO.end()) {
            pintar(renderer, color->second);
            renderer.FillRect(SDL2pp::Rect(caja.x + MARGEN_SIN_ICONO, caja.y + MARGEN_SIN_ICONO,
                                           caja.w - 2 * MARGEN_SIN_ICONO,
                                           caja.h - 2 * MARGEN_SIN_ICONO));
        }
        return;
    }
    const SDL2pp::Rect& recorte = icono->second.recorte;
    int lado = std::max(recorte.w, recorte.h);
    int ancho = lado > ICONO_MAXIMO ? recorte.w * ICONO_MAXIMO / lado : recorte.w;
    int alto = lado > ICONO_MAXIMO ? recorte.h * ICONO_MAXIMO / lado : recorte.h;
    renderer.Copy(icono->second.textura, recorte,
                  SDL2pp::Rect(caja.x + (caja.w - ancho) / 2, caja.y + (caja.h - alto) / 2, ancho,
                               alto));
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
