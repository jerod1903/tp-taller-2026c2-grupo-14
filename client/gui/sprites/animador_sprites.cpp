#include "client/gui/sprites/animador_sprites.h"

AnimadorSprites::AnimadorSprites(const FormatoAnimacion& formato): formato(formato) {}

void AnimadorSprites::actualizar(int8_t dir_x, int8_t dir_y,
                                 std::chrono::steady_clock::time_point ahora) {
    if (dir_x == 0 && dir_y == 0) {
        caminando = false;
        paso = 0;
        return;
    }
    if (!caminando) {
        caminando = true;
        inicio = ahora;
    }
    if (dir_x != 0) {
        mirando = dir_x > 0 ? Orientacion::Derecha : Orientacion::Izquierda;
    } else {
        mirando = dir_y > 0 ? Orientacion::Abajo : Orientacion::Arriba;
    }
    paso = static_cast<std::size_t>((ahora - inicio) / formato.duracion_cuadro) %
           formato.caminata.size();
}

Cuadro AnimadorSprites::cuadro() const {
    const auto& [fila, espejado] = formato.filas.at(mirando);
    int columna = formato.caminata[paso];
    auto bajada = formato.bajada_cabeza.find(columna);
    return Cuadro{columna, fila, espejado, mirando,
                  bajada == formato.bajada_cabeza.end() ? 0 : bajada->second};
}
