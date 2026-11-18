#ifndef SONIDOS_GRANJA_H
#define SONIDOS_GRANJA_H

#include <set>

#include "client/audio/sonido.h"
#include "common/farm_dto.h"

std::set<Sonido> sonidos_granja(const FarmState& antes, const FarmState& despues);

#endif
