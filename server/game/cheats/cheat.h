#ifndef CHEAT_H
#define CHEAT_H

#include <optional>
#include <string>

class Player;
class GameContext;

// Lo que pasa cuando un jugador escribe "$<nombre> [payload]" en el chat. Sin estado: hay uno
// por nombre.
class Cheat {
public:
    // payload: lo que vino después del nombre, sin espacios a los costados ("" si no vino nada).
    virtual void apply(Player& player, GameContext& context, const std::string& payload) const = 0;

    virtual ~Cheat() = default;

protected:
    // El payload como cantidad positiva, o `fallback` si vino vacío. nullopt si no es un número
    // positivo.
    static std::optional<int> amount(const std::string& payload, int fallback);
    // Le avisa por el chat, solo a él.
    static void tell(const Player& player, GameContext& context, const std::string& text);
};

// El cheat con ese nombre (lo que va después del $), o nullptr si no hay ninguno.
const Cheat* cheat_for(const std::string& name);

#endif  // CHEAT_H
