#include "client/gui/sprites/golpe_espada.h"

#include "client/gui/sprites/constantes_golpe_espada.h"

GolpeEspada golpe_espada_de(Orientacion mirando, float avance) {
    const auto& [angulo_medio, dx, dy] = POSICIONES_GOLPE_ESPADA.at(mirando);
    double angulo = angulo_medio + (avance - 0.5) * BARRIDO_GOLPE_ESPADA;
    return GolpeEspada{angulo, dx, dy, mirando == Orientacion::Arriba};
}
