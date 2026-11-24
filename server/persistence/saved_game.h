#ifndef SAVED_GAME_H
#define SAVED_GAME_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "common/farm_dto.h"
#include "common/game_dto.h"
#include "server/game/characters/animals/animal_kind.h"
#include "server/game/world/position.h"

// Lo que se guarda de una partida al pasar de día: el estado del día nuevo, recién despiertos.
// Es del servidor (no viaja al cliente) y no sabe en qué formato se escribe: cada clase del modelo
// arma y restaura su parte, y server/persistence lo escribe y lo lee del archivo.
// Los cofres se guardan con su contenido. No se guardan las posiciones de los jugadores (al cargar
// despiertan en su cama), las camas, la mazmorra ni el chat. La apariencia y el nombre de cada
// jugador están en el índice (GameRecord).

// Un cultivo con todo lo que hace falta para seguir creciendo y para la calidad al cosecharlo.
struct SavedCrop {
    CropType type = CropType::None;
    int days_grown = 0;
    int window = 0;
    bool watered = false;
    float window_start = 0.0f;
    int skipped = 0;
    float watering_delay = 0.0f;
    int waterings = 0;
    int skipped_total = 0;
    int days_ready = 0;

    bool operator==(const SavedCrop&) const = default;
};

// Una celda trabajada de la granja
struct SavedPlot {
    SoilType soil = SoilType::Dirt;
    SavedCrop crop;

    bool operator==(const SavedPlot&) const = default;
};

// Un animal: dónde está (su celda, para no apilarlos al cargar) y cómo va su producción. Cada
// especie usa lo suyo: la vaca days_left y ready; la gallina days_left, lay_in y laid.
struct SavedAnimal {
    AnimalKind kind = AnimalKind::Cow;
    Position cell{0, 0};
    int days_left = 0;
    bool ready = false;
    std::optional<float> lay_in;
    bool laid = false;

    bool operator==(const SavedAnimal&) const = default;
};

// Un cofre: en qué área (la granja o la cabaña) y celda está, y lo que tiene.
struct SavedChest {
    std::string area;
    Position cell{0, 0};
    std::vector<InventorySlot> slots;

    bool operator==(const SavedChest&) const = default;
};

struct SavedFarm {
    std::map<Position, SavedPlot> plots;
    std::map<Position, ItemType> ground_items;
    std::vector<SavedAnimal> animals;
    std::vector<SavedChest> chests;

    bool operator==(const SavedFarm&) const = default;
};

struct SavedPlayer {
    int gold = 0;
    int health = 0;
    int energy = 0;
    InventoryState inventory;
    uint32_t pending = 0;  // premios del centro comunitario que todavía no cobró

    bool operator==(const SavedPlayer&) const = default;
};

struct SavedGame {
    uint16_t day = 1;
    SavedFarm farm;
    // Por playerId: todos los que alguna vez entraron a la partida
    std::map<uint32_t, SavedPlayer> players;

    bool operator==(const SavedGame&) const = default;
};

#endif  // SAVED_GAME_H
