#ifndef POSE_H
#define POSE_H

#include <map>

#include "client/gui/sprites/animador_sprites.h"
#include "common/game_dto.h"

// La hoja con la que se dibuja una entidad que tiene una por acción (un esqueleto).
enum class Pose { Quieto, Caminando, Atacando, Herido, Muerto };

// El PNG de una pose y cómo se recorta. `corrimiento`: cuántos píxeles se corre el dibujo desde
// donde iría (centrado en x, con la base en los pies), para las que no se apoyan en el centro de
// su celda (una trampa).
struct HojaDePose {
    const char* ruta;
    FormatoAnimacion formato;
    int corrimiento_x = 0;
    int corrimiento_y = 0;
};

// La acción que manda el servidor; sin ninguna, camina o está quieto según su dirección.
Pose pose_de(const EntityState& estado);

// Para las que tienen menos hojas que poses: `base` en todas las poses, salvo las que estén en
// `distintas`. Una calavera se ve igual haga lo que haga (sin `distintas`); una trampa solo cambia
// al activarse ({{Pose::Atacando, activa}}).
std::map<Pose, HojaDePose> hojas_de(const HojaDePose& base,
                                    const std::map<Pose, HojaDePose>& distintas = {});

#endif
