#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <cstdint>
#include <string>

// Lo que se ajusta del juego sin recompilar. Se lee de server_config.toml, en el root del repo.
struct ServerConfig {
    struct Game {
        uint8_t max_players;
        int tick_rate;  // ticks por segundo
        unsigned int outbox_size;
    };
    struct Player {
        int health;
        int energy;
        int gold;
        float speed;  // celdas por segundo
        // Los objetos iguales se apilan en un slot del inventario hasta esta cantidad.
        uint8_t max_stack;
    };
    struct Animals {
        int cows;
        int chickens;
        // Donde arranca la fila de animales, respecto de donde aparece el jugador
        int offset_x;
        int offset_y;
        int health;
        float speed;     // celdas por segundo
        float min_walk;  // segundos
        float max_walk;
        // Provisorio, hasta que exista el ciclo de días: cada cuánto da leche una vaca, en
        // segundos.
        float cow_ready_seconds;
        // Hasta dónde llega el jugador para ordeñar: de su centro al de la vaca, en casillas.
        float milking_reach;
    };

    Game game;
    Player player;
    Animals animals;
};

// Lanza std::runtime_error si el archivo no se puede leer, le falta un valor o uno es inválido.
ServerConfig load_server_config(const std::string& path);

// La de server_config.toml, que se lee una sola vez.
const ServerConfig& server_config();

#endif  // SERVER_CONFIG_H
