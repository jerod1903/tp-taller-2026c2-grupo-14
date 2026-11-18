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

    // Lo que comparten los enemigos que pelean cuerpo a cuerpo
    struct Fighter {
        int health;
        int damage;      // vida que le saca a quien golpea
        float speed;     // celdas por segundo
        float sight;     // hasta donde ve a un jugador, en celdas
        float reach;     // hasta donde llega su golpe: de su centro al del jugador, en celdas
        float cooldown;  // segundos entre golpe y golpe
    };
    // Tiene que estar antes de Enemies, que la usa
    struct Skeleton {
        int count;  // cuantos aparecen en cada nivel
        Fighter fighter;
        float revive_seconds;  // cuanto tarda en revivir despues de morir
    };
    struct Enemies {
        // Al aparecer, ninguno queda a menos de esta distancia (en celdas) de un portal del nivel:
        // el que entra no se los encuentra encima.
        float spawn_distance;
        Skeleton skeleton;
    };
    // Los comerciantes: hasta dónde se les habla, cuánto cansa y los precios de lo que venden
    // (price) y de lo que compran (buyback).
    struct Market {
        float reach;  // casillas, del centro del jugador al de la celda del comerciante
        int talk_energy;
        int cow_price;
        int cow_buyback;
        int chicken_price;
        int chicken_buyback;
        int milk_buyback;
        int max_cows;  // tope por granja
        int max_chickens;
    };

    Game game;
    Player player;
    Animals animals;
    Enemies enemies;
    Market market;
};

// Lanza std::runtime_error si el archivo no se puede leer, le falta un valor o uno es inválido.
ServerConfig load_server_config(const std::string& path);

// La de server_config.toml, que se lee una sola vez.
const ServerConfig& server_config();

#endif  // SERVER_CONFIG_H
