#ifndef PLAYER_STORAGE_H
#define PLAYER_STORAGE_H

#include <cstdint>
#include <optional>
#include <vector>

#include "common/game_dto.h"
#include "server/game/world/position.h"

class Player;

// Cómo quedó un cofre después de un cambio, y a quiénes mandárselo: todos los que lo tienen
// abierto.
struct ChestUpdate {
    ChestState chest;
    std::vector<uint32_t> viewers;
};

// Abrir cofres y mover objetos entre el inventario del jugador y el cofre que tiene abierto: lo
// que usan ClickCommand y los comandos del cofre.
class PlayerStorage {
public:
    // Abre el cofre más cercano del área del jugador, si alguno está a su alcance (chest_reach), y
    // devuelve su contenido. Si no, nullopt.
    virtual std::optional<ChestState> open(Player& player) = 0;
    // Pasa la pila entera de `slot` de `from` al otro inventario. nullopt si no tiene un cofre
    // abierto a su alcance, `from` es inválido, es una herramienta de trabajo del jugador o no se
    // movió nada.
    virtual std::optional<ChestUpdate> transfer(Player& player, ChestSide from, uint8_t slot) = 0;
    // Parte la pila de `slot` de `side` (la mitad al primer slot vacío del mismo inventario).
    // nullopt si no tiene un cofre abierto a su alcance, `side` es inválido o no se pudo partir.
    virtual std::optional<ChestUpdate> split(Player& player, ChestSide side, uint8_t slot) = 0;
    // Deja de tener abierto su cofre; si no tenía ninguno, no hace nada.
    virtual void close(const Player& player) = 0;

    virtual ~PlayerStorage() = default;
};

#endif  // PLAYER_STORAGE_H
