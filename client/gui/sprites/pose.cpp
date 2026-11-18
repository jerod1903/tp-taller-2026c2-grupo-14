#include "client/gui/sprites/pose.h"

Pose pose_de(const EntityState& estado) {
    switch (estado.action) {
        case EntityAction::Dead:
            return Pose::Muerto;
        case EntityAction::Hurt:
            return Pose::Herido;
        case EntityAction::Attacking:
            return Pose::Atacando;
        case EntityAction::None:
            break;
    }
    return estado.dirX == 0 && estado.dirY == 0 ? Pose::Quieto : Pose::Caminando;
}
