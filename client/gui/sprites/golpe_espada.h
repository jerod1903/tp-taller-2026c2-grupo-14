#ifndef GOLPE_ESPADA_H
#define GOLPE_ESPADA_H

#include "client/gui/sprites/animador_sprites.h"

struct GolpeEspada {
    double angulo;
    int dx;
    int dy;
    bool detras;
};

GolpeEspada golpe_espada_de(Orientacion mirando, float avance);

#endif
