#ifndef CREATURE_H
#define CREATURE_H

#include <cstdint>

#include "server/game/characters/entity.h"
#include "server/game/world/direction.h"
#include "server/game/world/point.h"

// Una entidad con vida que camina libremente, no de a una celda: (x, y) es su centro, en celdas y
// con decimales. `pos` es la celda en la que está ese centro, la que usan portales y herramientas.
class Creature: public Entity {
public:
    // `damage`: la vida que le saca a quien ataca. Las que no pelean (los animales) no la usan.
    Creature(uint32_t id, const Position& pos, Location& location, int health, float base_speed,
             int damage = 0);

    int get_health() const;

    virtual void take_damage(int amount);
    bool is_alive() const;
    // Muerta o gravemente herida. Por defecto, solo muerta; el jugador también con poca vida.
    virtual bool is_badly_hurt() const;
    // Le pega a `target`: le saca attack_damage(). Cuándo pegar lo decide quien la maneja (la IA
    // del enemigo, la acción del jugador).
    void attack(Creature& target) const;

    // Celdas por segundo. Por defecto base_speed; las subclases pueden modificarla.
    virtual float speed() const;

    // Hacia dónde camina; NONE frena en el acto. En diagonal va a la misma velocidad.
    void set_direction(Direction dir);
    Direction get_direction() const;
    // Hacia dónde mira: la última dirección en la que caminó (en diagonal, el costado). Arranca
    // mirando abajo.
    Direction get_facing() const;
    // La celda que tiene adelante, donde usa la herramienta.
    Position facing_cell() const;
    // El centro, en celdas, para que el cliente lo dibuje donde está.
    float get_x() const;
    float get_y() const;

    // true si en el último advance() entró a una celda nueva. Los portales se disparan al entrar,
    // no por estar parado encima: si no, quien llega sobre un portal de vuelta rebota.
    bool has_stepped() const;
    // true si su caja de colisión pisa una celda que no se puede pisar (por ejemplo, si apareció
    // una cama donde estaba parado).
    bool is_trapped() const;

    // Llega parado en el centro de `arrival`.
    void relocate(Location& target, const Position& arrival) override;
    // Pone su centro en `point`, dentro de la misma Location, aunque no caiga en el centro de una
    // celda.
    void place_at(const Point& point);

protected:
    int health;
    float base_speed;
    int damage;
    Direction direction = Direction::NONE;
    Direction facing = Direction::DOWN;
    float x;
    float y;
    bool stepped = false;

    // La vida que le saca a quien ataca. Por defecto su daño; las subclases pueden modificarla.
    virtual int attack_damage() const;

    // Mueve el centro speed() * dt hacia `direction`. Contra una pared se queda pegado a ella y
    // sigue por el eje que esté libre. La llama el update() de cada subclase.
    // true si cambió de celda en este tick.
    bool advance(float dt);

private:
    // Mueve sobre un eje hasta donde lo dejen las paredes
    void move_x(float dx);
    void move_y(float dy);
    // Si la caja de colisión centrada en (cx, cy) solo pisa celdas caminables
    bool fits(float cx, float cy) const;
};

#endif  // CREATURE_H
