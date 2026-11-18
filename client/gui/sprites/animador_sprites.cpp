#include "client/gui/sprites/animador_sprites.h"

#include <algorithm>
#include <numeric>
#include <vector>

FormatoAnimacion formato_tira(int ancho_cuadro, int alto_cuadro, int cuadros,
                              std::chrono::duration<float> duracion_cuadro, bool repite) {
    std::vector<int> secuencia(cuadros);
    std::iota(secuencia.begin(), secuencia.end(), 0);
    FormatoAnimacion formato{ancho_cuadro,
                             alto_cuadro,
                             {{Orientacion::Derecha, {0, false}},
                              {Orientacion::Izquierda, {0, true}},
                              {Orientacion::Abajo, {0, false}},
                              {Orientacion::Arriba, {0, false}}},
                             secuencia,
                             duracion_cuadro};
    formato.anima_quieto = true;
    formato.repite = repite;
    return formato;
}

AnimadorSprites::AnimadorSprites(const FormatoAnimacion& formato): formato(formato) {}

void AnimadorSprites::actualizar(int8_t dir_x, int8_t dir_y,
                                 std::chrono::steady_clock::time_point ahora) {
    bool quieto = dir_x == 0 && dir_y == 0;
    if (quieto && !formato.anima_quieto) {
        animando = false;
        paso = 0;
        return;
    }
    if (!animando) {
        animando = true;
        inicio = ahora;
    }
    if (dir_x != 0) {
        mirando = dir_x > 0 ? Orientacion::Derecha : Orientacion::Izquierda;
    } else if (dir_y != 0) {
        mirando = dir_y > 0 ? Orientacion::Abajo : Orientacion::Arriba;
    }
    auto pasados = static_cast<std::size_t>((ahora - inicio) / formato.duracion_cuadro);
    std::size_t cuadros = formato.caminata.size();
    paso = formato.repite ? pasados % cuadros : std::min(pasados, cuadros - 1);
}

void AnimadorSprites::cambiar_formato(const FormatoAnimacion& nuevo) {
    formato = nuevo;
    animando = false;
    paso = 0;
}

Cuadro AnimadorSprites::cuadro() const {
    const auto& [fila, espejado] = formato.filas.at(mirando);
    int columna = formato.caminata[paso];
    auto bajada = formato.bajada_cabeza.find(columna);
    return Cuadro{columna, fila, espejado, mirando,
                  bajada == formato.bajada_cabeza.end() ? 0 : bajada->second};
}
