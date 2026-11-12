#ifndef ENTITY_STATE_H
#define ENTITY_STATE_H

#include <cstdint>
#include <string>

// PROVISORIO: copia de lo que se le pidio al protocolo para common/game_dto.h, con los mismos
// nombres. Cuando llegue el DTO real se borra este archivo y se incluye game_dto.h

// Que es cada cosa que se dibuja y se mueve
enum class EntityType : uint8_t { Player, Cow, Chicken };

struct EntityState {
    uint32_t entityId;  // unico dentro de su tipo; si type == Player, es el playerId
    EntityType type;
    std::string areaId;
    uint32_t x;  // centro, en milesimas de casilla (como PlayerState)
    uint32_t y;
    int8_t dirX;  // hacia donde camina, -1/0/1; (0, 0) quieto
    int8_t dirY;
};

#endif
