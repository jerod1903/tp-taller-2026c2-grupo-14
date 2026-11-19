#ifndef FILE_GAME_STORE_H
#define FILE_GAME_STORE_H

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "server/persistence/game_store.h"

// Las partidas en archivos TOML dentro de `directory` (que crea si no existe): el índice en
// index.toml y el estado de cada partida en game_<id>.toml. Cada archivo se reescribe entero al
// guardar: primero en uno temporal y después se renombra, así un corte a mitad no deja uno roto.
// Un archivo mal formado lanza std::runtime_error al leerlo.
//
// Hilos:
// - El índice (load_all, save) lo usan las sesiones del lobby, cada una en su hilo, a través del
//   GameRegistry. `games` y index.toml (y su .tmp) son compartidos, y save() tiene que ser atómico
//   entero (actualizar el mapa, armar el índice y escribirlo): si no, dos save() a la vez pueden
//   pisarse el .tmp o escribir una copia vieja del índice y perder una partida. Lo protege `mutex`.
//   El registry ya llama a save() con su propio lock tomado, pero el store no depende de eso: se
//   protege solo. No hay riesgo de deadlock: el registry toma su lock y después el del store, y el
//   store nunca llama al registry.
// - El estado de una partida (load_game, save_game) lo usa solo el Game de esa partida, en su hilo,
//   y cada partida tiene su propio archivo: no comparten memoria ni archivos, así que no llevan
//   lock.
class FileGameStore: public GameStore {
public:
    explicit FileGameStore(std::string directory);

    // El índice. Si todavía no hay ninguno, vacío.
    std::vector<GameRecord> load_all() override;
    void save(const GameRecord& game) override;
    std::optional<SavedGame> load_game(uint32_t game_id) override;
    void save_game(uint32_t game_id, const SavedGame& game) override;

private:
    std::string directory;
    // El índice en memoria: save() lo actualiza y reescribe el archivo entero
    std::map<uint32_t, GameRecord> games;
    // Protege `games` e index.toml
    std::mutex mutex;

    std::string index_path() const;
    std::string game_path(uint32_t game_id) const;
};

#endif  // FILE_GAME_STORE_H
