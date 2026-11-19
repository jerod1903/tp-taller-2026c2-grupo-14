#include "client/gui/sprites/pose.h"

#include <map>

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

std::map<Pose, HojaDePose> hojas_de(const HojaDePose& base,
                                    const std::map<Pose, HojaDePose>& distintas) {
    std::map<Pose, HojaDePose> hojas = distintas;
    for (Pose pose: {Pose::Quieto, Pose::Caminando, Pose::Atacando, Pose::Herido, Pose::Muerto}) {
        hojas.try_emplace(pose, base);
    }
    return hojas;
}
