#include "client/gui/hud/marco.h"

#include "client/constants.h"

void dibujar_marco(SDL2pp::Renderer& renderer, SDL2pp::Texture& marco, const SDL2pp::Rect& destino,
                   int escala) {
    SDL2pp::Point origen = marco.GetSize();
    int borde = BORDE_MARCO;
    int borde_destino = BORDE_MARCO * escala;
    // Por eje: el borde de un lado, el medio que se estira y el borde del otro
    const int origen_x[] = {0, borde, origen.x - borde};
    const int origen_w[] = {borde, origen.x - 2 * borde, borde};
    const int origen_y[] = {0, borde, origen.y - borde};
    const int origen_h[] = {borde, origen.y - 2 * borde, borde};
    const int destino_x[] = {destino.x, destino.x + borde_destino,
                             destino.x + destino.w - borde_destino};
    const int destino_w[] = {borde_destino, destino.w - 2 * borde_destino, borde_destino};
    const int destino_y[] = {destino.y, destino.y + borde_destino,
                             destino.y + destino.h - borde_destino};
    const int destino_h[] = {borde_destino, destino.h - 2 * borde_destino, borde_destino};
    for (int fila = 0; fila < 3; ++fila) {
        for (int columna = 0; columna < 3; ++columna) {
            renderer.Copy(marco,
                          SDL2pp::Rect(origen_x[columna], origen_y[fila], origen_w[columna],
                                       origen_h[fila]),
                          SDL2pp::Rect(destino_x[columna], destino_y[fila], destino_w[columna],
                                       destino_h[fila]));
        }
    }
}
