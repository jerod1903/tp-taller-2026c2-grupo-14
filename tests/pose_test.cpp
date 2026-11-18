#include "client/gui/sprites/pose.h"

#include <gtest/gtest.h>

namespace {

EntityState esqueleto(int8_t dir_x, EntityAction accion) {
    return EntityState{1000, EntityType::Skeleton, "nivel", 0, 0, dir_x, 0, Emote::None, accion};
}

TEST(PoseTest, SinAccionCaminaOEstaQuietoSegunSuDireccion) {
    EXPECT_EQ(pose_de(esqueleto(0, EntityAction::None)), Pose::Quieto);
    EXPECT_EQ(pose_de(esqueleto(1, EntityAction::None)), Pose::Caminando);
}

TEST(PoseTest, CadaAccionTieneSuPose) {
    EXPECT_EQ(pose_de(esqueleto(0, EntityAction::Attacking)), Pose::Atacando);
    EXPECT_EQ(pose_de(esqueleto(0, EntityAction::Hurt)), Pose::Herido);
    EXPECT_EQ(pose_de(esqueleto(0, EntityAction::Dead)), Pose::Muerto);
}

TEST(PoseTest, LaAccionTapaQueEsteCaminando) {
    EXPECT_EQ(pose_de(esqueleto(1, EntityAction::Hurt)), Pose::Herido);
}

}  // namespace
