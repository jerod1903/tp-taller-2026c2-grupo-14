#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <cstddef>
#include <cstdint>

#include "common/lobby_dto.h"

// UUID v4 válido y determinístico: semillas distintas dan usuarios distintos.
inline UserId make_user_id(uint8_t seed = 0xA0) {
    UserId id{};
    for (std::size_t i = 0; i < id.size(); ++i) {
        id[i] = static_cast<uint8_t>(seed + i);
    }
    id[6] = static_cast<uint8_t>(0x40 | (id[6] & 0x0F));
    id[8] = static_cast<uint8_t>(0x80 | (id[8] & 0x3F));
    return id;
}

#endif  // TEST_HELPERS_H
