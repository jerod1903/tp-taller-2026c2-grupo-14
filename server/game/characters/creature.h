#ifndef CREATURE_H
#define CREATURE_H

#include <cstdint>

#include "server/game/characters/entity.h"
#include "server/game/world/direction.h"

// Una entidad con vida que se mueve de a una celda. La posición lógica es siempre entera: el
// avance parcial hacia la celda siguiente se acumula en `progress` y el cliente interpola.
class Creature: public Entity {
public:
    Creature(uint32_t id, const Position& pos, Location& location, int health, float base_speed);

    int get_health() const;

    // La vida no baja de 0.
    virtual void take_damage(int amount);
    bool is_alive() const;

    // Celdas por segundo. Por defecto base_speed; las subclases pueden modificarla.
    virtual float speed() const;

    // Hacia dónde camina; NONE frena. Cambiar de dirección reinicia `progress`.
    void set_direction(Direction dir);
    Direction get_direction() const;

    // true si en el último advance() entró a una celda nueva. Los portales se disparan al entrar,
    // no por estar parado encima: si no, quien llega sobre un portal de vuelta rebota.
    bool has_stepped() const;

protected:
    int health;
    float base_speed;
    Direction direction = Direction::NONE;
    float progress = 0.0f;  // 0..1 hacia step(pos, direction)
    bool stepped = false;

    // Suma speed() * dt a `progress`. Al llegar a 1 pasa a la celda siguiente si la Location
    // dice que es caminable; si no, frena. La llama el update() de cada subclase.
    // true si cambió de celda en este tick.
    bool advance(float dt);
};

#endif  // CREATURE_H
