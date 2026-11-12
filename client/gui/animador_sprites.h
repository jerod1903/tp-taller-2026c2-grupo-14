#ifndef ANIMADOR_SPRITES_H
#define ANIMADOR_SPRITES_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

enum class Orientacion { Abajo, Arriba, Izquierda, Derecha };

// Que cuadro de la hoja mostrar, contado en cuadros (no en pixeles)
struct Cuadro {
    int columna;
    int fila;
    bool espejado;
    Orientacion orientacion;  // las capas por vista eligen su imagen con esto
    // En pixeles: en los pasos la cabeza baja, y lo que lleva encima tambien
    int bajada_cabeza;

    bool operator==(const Cuadro&) const = default;
};

// Como esta armada la hoja de un tipo de entidad: es lo unico que cambia entre un personaje, una
// vaca o una gallina
struct FormatoAnimacion {
    int ancho_cuadro;  // en pixeles
    int alto_cuadro;
    // orientacion -> (fila de la hoja, si se espeja): las hojas traen un solo costado
    std::map<Orientacion, std::pair<int, bool>> filas;
    // Columnas de la caminata, en orden; quieto muestra la primera
    std::vector<int> caminata;
    std::chrono::duration<float> duracion_cuadro;
    // columna -> cuantos pixeles baja la cabeza en ese cuadro; las que no estan, no baja
    std::map<int, int> bajada_cabeza = {};
};

// Decide que cuadro mostrar segun hacia donde camina la entidad y cuanto hace que camina. No usa
// SDL: lo que se dibuja lo arma EntidadGrafica
class AnimadorSprites {
public:
    explicit AnimadorSprites(const FormatoAnimacion& formato);
    // Se llama en cada frame con la direccion del ultimo Snapshot. (0, 0) es quieto: muestra el
    // primer cuadro y sigue mirando hacia donde iba. En diagonal mira de costado
    void actualizar(int8_t dir_x, int8_t dir_y, std::chrono::steady_clock::time_point ahora);
    Cuadro cuadro() const;

private:
    FormatoAnimacion formato;
    Orientacion mirando = Orientacion::Abajo;
    bool caminando = false;
    std::chrono::steady_clock::time_point inicio;  // cuando empezo a caminar
    std::size_t paso = 0;                          // indice en formato.caminata
};

#endif
