#ifndef POSE_H
#define POSE_H

#include <map>

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

// Para las que se ven igual hagan lo que hagan (una calavera, una llama): la misma hoja en todas
// las poses.
std::map<Pose, HojaDePose> misma_hoja(const HojaDePose& hoja);

#endif
