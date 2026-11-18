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
    int margen_arriba = 0;
    // Si pasa los cuadros aunque esté quieto (un esqueleto respirando, pegando): si no, quieto
    // muestra el primero
    bool anima_quieto = false;
    // Si al terminar vuelve a empezar; si no, se queda en el último cuadro (morir)
    bool repite = true;
};

// Una tira: una sola fila de `cuadros` cuadros mirando a la derecha (a la izquierda se espeja),
// que se anima aunque esté quieta.
FormatoAnimacion formato_tira(int ancho_cuadro, int alto_cuadro, int cuadros,
                              std::chrono::duration<float> duracion_cuadro, bool repite = true);

class AnimadorSprites {
public:
    explicit AnimadorSprites(const FormatoAnimacion& formato);
    void actualizar(int8_t dir_x, int8_t dir_y, std::chrono::steady_clock::time_point ahora);
    // Pasa a otra hoja (otra pose) desde su primer cuadro, mirando para el mismo lado
    void cambiar_formato(const FormatoAnimacion& nuevo);
    Cuadro cuadro() const;

private:
    FormatoAnimacion formato;
    Orientacion mirando = Orientacion::Abajo;
    bool animando = false;
    std::chrono::steady_clock::time_point inicio;
    std::size_t paso = 0;
};

#endif
