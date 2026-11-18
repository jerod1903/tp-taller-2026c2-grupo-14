#ifndef OUTCOME_H
#define OUTCOME_H

class Player;
class GameContext;

// Cómo terminó una interacción, y qué pasa después. Sin estado: hay una de cada una.
class Outcome {
public:
    // false si no había con qué hacerla y hay que probar la siguiente.
    virtual bool apply(Player& player, GameContext& context, int energy_cost) const = 0;

    virtual ~Outcome() = default;

    // No había con qué hacerla al alcance.
    static const Outcome& not_here();
    // No le entraba en el inventario: se le avisa por el chat.
    static const Outcome& inventory_full();
    // Se hizo: cansa y se le manda el inventario nuevo.
    static const Outcome& done();
};

#endif  // OUTCOME_H
