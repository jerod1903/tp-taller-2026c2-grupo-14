#ifndef POSE_H
#define POSE_H

#include "client/gui/sprites/animador_sprites.h"
#include "common/game_dto.h"

// La hoja con la que se dibuja una entidad que tiene una por acción (un esqueleto).
enum class Pose { Quieto, Caminando, Atacando, Herido, Muerto };

// El PNG de una pose y cómo se recorta.
struct HojaDePose {
    const char* ruta;
    FormatoAnimacion formato;
};

// La acción que manda el servidor; sin ninguna, camina o está quieto según su dirección.
Pose pose_de(const EntityState& estado);

#endif
