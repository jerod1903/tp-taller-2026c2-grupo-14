#ifndef ANIMADOR_SPRITES_H
#define ANIMADOR_SPRITES_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

enum class Orientacion { Abajo, Arriba, Izquierda, Derecha };

struct Cuadro {
    int columna;
    int fila;
    bool espejado;
    Orientacion orientacion;
    int bajada_cabeza;

    bool operator==(const Cuadro&) const = default;
};

struct FormatoAnimacion {
    int ancho_cuadro;
    int alto_cuadro;
    std::map<Orientacion, std::pair<int, bool>> filas;
    std::vector<int> caminata;
    std::chrono::duration<float> duracion_cuadro;
    std::map<int, int> bajada_cabeza = {};
};

class AnimadorSprites {
public:
    explicit AnimadorSprites(const FormatoAnimacion& formato);
    void actualizar(int8_t dir_x, int8_t dir_y, std::chrono::steady_clock::time_point ahora);
    Cuadro cuadro() const;

private:
    FormatoAnimacion formato;
    Orientacion mirando = Orientacion::Abajo;
    bool caminando = false;
    std::chrono::steady_clock::time_point inicio;
    std::size_t paso = 0;
};

#endif
