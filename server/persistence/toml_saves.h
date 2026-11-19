#ifndef TOML_SAVES_H
#define TOML_SAVES_H

#include <vector>

#include <toml++/toml.hpp>

#include "server/persistence/game_store.h"
#include "server/persistence/saved_game.h"

// La conversión entre lo que se guarda y TOML, sin archivos (de eso se ocupa FileGameStore).
// Los enums se escriben como su número: agregar valores al final de un enum no rompe partidas
// guardadas, pero reordenarlos sí. Leer algo mal formado lanza std::runtime_error.

// El índice: las partidas, cada una con sus miembros.
toml::table index_to_toml(const std::vector<GameRecord>& games);
std::vector<GameRecord> index_from_toml(const toml::table& table);

// Una partida.
toml::table saved_game_to_toml(const SavedGame& game);
SavedGame saved_game_from_toml(const toml::table& table);

#endif  // TOML_SAVES_H
